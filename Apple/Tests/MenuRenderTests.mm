#import <XCTest/XCTest.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import "../../Sources/Platform/Apple/MDRenderer.h"
#include "../../Sources/Core/Menu.hpp"
#include <algorithm>
#include <stdexcept>

namespace {
class MenuResources final:public md::ResourceAccess {
    NSURL* root_;
    MDRenderer* renderer_;
public:
    MenuResources(NSURL* root,MDRenderer* renderer):root_(root),renderer_(renderer){}
    md::Bytes read(std::string_view path) override {
        NSString* relative=[[NSString alloc] initWithBytes:path.data() length:path.size() encoding:NSUTF8StringEncoding];
        NSData* data=[NSData dataWithContentsOfURL:[root_ URLByAppendingPathComponent:relative]];
        if(!data)throw std::runtime_error("Missing menu resource: "+std::string(path));
        const auto* start=static_cast<const std::uint8_t*>(data.bytes);
        return {start,start+data.length};
    }
    md::Vec2 imageSize(std::string_view name)override{return [renderer_ imageSize:std::string(name)];}
};
class MenuHost final:public md::HostServices {
public:
    unsigned saves=0,names=0;
    md::Bytes saved;
    std::optional<md::Bytes> readSave()override{return saved.empty()?std::nullopt:std::optional(saved);}
    bool writeSave(std::span<const std::uint8_t> value)override{saved.assign(value.begin(),value.end());++saves;return true;}
    void playSound(std::string_view,float,float)override{}
    void playMusic(std::string_view,float)override{}
    void stopMusic()override{}
    void requestName(std::string,std::size_t maximum,std::function<void(std::optional<std::string>)> done)override{
        ++names;if(maximum<7)throw std::runtime_error("Unexpected profile capacity");done("PLAYER1");
    }
};
}
@interface MDMenuSurface:UIView @end
@implementation MDMenuSurface
+(Class)layerClass{return CAEAGLLayer.class;}
@end
@interface MenuRenderTests:XCTestCase @end
@implementation MenuRenderTests
- (void)capture:(NSString*)name renderer:(MDRenderer*)renderer {
    NSData* raw=[renderer readFramebufferRGBA];CGSize size=renderer.drawablePixelSize;
    const auto width=NSUInteger(size.width),height=NSUInteger(size.height),stride=width*4;
    XCTAssertEqual(raw.length,stride*height);if(raw.length!=stride*height)return;
    NSMutableData* topDown=[NSMutableData dataWithLength:raw.length];
    for(NSUInteger y=0;y<height;++y)memcpy(static_cast<char*>(topDown.mutableBytes)+y*stride,static_cast<const char*>(raw.bytes)+(height-1-y)*stride,stride);
    CGDataProviderRef provider=CGDataProviderCreateWithCFData((__bridge CFDataRef)topDown);
    CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
    CGImageRef cg=CGImageCreate(width,height,8,32,stride,space,kCGImageAlphaLast|kCGBitmapByteOrder32Big,provider,nullptr,false,kCGRenderingIntentDefault);
    UIImage* image=[UIImage imageWithCGImage:cg];
    XCTAttachment* attachment=[XCTAttachment attachmentWithImage:image];attachment.name=name;attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
    CGImageRelease(cg);CGColorSpaceRelease(space);CGDataProviderRelease(provider);
}
- (void)testOriginalMenuFramesAndEnglishProfileGlyphs {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:CGRectMake(0,0,320,480)];surface.contentScaleFactor=1;surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];XCTAssertNotNil(renderer);
    MenuResources resources(root,renderer);MenuHost host;auto save=md::SaveData::empty(1);md::Menu menu(resources,host,save);
    for(unsigned i=0;i<30;++i)menu.tick();
    XCTAssertTrue([renderer drawFrame:menu.frame() toLayer:layer],@"%@",renderer.lastError);
    [self capture:@"01-original-title" renderer:renderer];XCTAssertEqual(host.saves,0u);
    menu.touch({1,md::TouchPhase::ended,{160,240}});XCTAssertEqual(menu.screen(),md::Menu::Screen::profiles);
    menu.touch({2,md::TouchPhase::began,{240,450}});menu.touch({2,md::TouchPhase::ended,{240,450}});
    XCTAssertEqual(host.names,1u);XCTAssertEqual(host.saves,1u);XCTAssertTrue(save.active(0));XCTAssertEqual(save.name(0),std::string("PLAYER1"));
    XCTAssertTrue([renderer drawFrame:menu.frame() toLayer:layer],@"%@",renderer.lastError);
    [self capture:@"02-created-English-profile" renderer:renderer];
    const auto glyphCount=std::count_if(menu.frame().sprites.begin(),menu.frame().sprites.end(),[](const md::Sprite& s){return s.image=="Generated/TrebuchetMS-Bold-48.png";});
    XCTAssertGreaterThanOrEqual(glyphCount,14);
    menu.touch({3,md::TouchPhase::began,{160,140}});menu.touch({3,md::TouchPhase::ended,{160,140}});
    XCTAssertEqual(menu.screen(),md::Menu::Screen::missions);XCTAssertEqual(host.saves,2u);
    for(unsigned i=0;i<90;++i)menu.tick();
    XCTAssertTrue([renderer drawFrame:menu.frame() toLayer:layer],@"%@",renderer.lastError);
    [self capture:@"03-original-stage-selection" renderer:renderer];
    const auto uploads=renderer.textureUploads;
    for(unsigned i=0;i<30;++i){menu.tick();XCTAssertTrue([renderer drawFrame:menu.frame() toLayer:layer]);}
    XCTAssertEqual(renderer.textureUploads,uploads);XCTAssertEqual(host.saves,2u);
    XCTAssertEqual(md::SaveData::decode(host.saved).name(0),std::string("PLAYER1"));
    [renderer finish];window.hidden=YES;
}
@end
