#import <XCTest/XCTest.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import "../../Sources/Platform/Apple/MDRenderer.h"
#include "../../Sources/Core/Menu.hpp"
#include "../../Sources/Core/GameSession.hpp"
#include "../../Sources/Core/World.hpp"
#include "../../Tests/ContentSweep.hpp"
#import "../../Sources/Platform/Apple/MDAudio.h"
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
// Isolated disk save and the production OpenAL backend. Never opens app saves.
class SweepHost final:public md::HostServices {
    NSURL* file_; MDAudio* audio_;
public:
    unsigned writes=0;
    SweepHost(NSURL* file,MDAudio* audio):file_(file),audio_(audio){}
    std::optional<md::Bytes> readSave()override {
        NSData* d=[NSData dataWithContentsOfURL:file_];if(!d)return {};
        const auto* p=static_cast<const std::uint8_t*>(d.bytes);return md::Bytes(p,p+d.length);
    }
    bool writeSave(std::span<const std::uint8_t> b)override {
        NSData* d=[NSData dataWithBytes:b.data() length:b.size()];
        if(![d writeToURL:file_ options:NSDataWritingAtomic error:nil])return false;
        ++writes;return true;
    }
    void playSound(std::string_view n,float gain,float pan)override {
        NSString* name=[[NSString alloc]initWithBytes:n.data() length:n.size() encoding:NSUTF8StringEncoding];
        [audio_ play:name gain:gain*.05f pan:pan];
    }
    void playMusic(std::string_view n,float gain)override {
        NSString* name=[[NSString alloc]initWithBytes:n.data() length:n.size() encoding:NSUTF8StringEncoding];
        [audio_ playMusic:name gain:gain*.05f];
    }
    void stopMusic()override{[audio_ stopMusic];}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)> done)override{done("SWEEP");}
};
void sweepTap(md::GameSession& s,float x,float y){s.touch({1,md::TouchPhase::began,{x,y}});s.touch({1,md::TouchPhase::ended,{x,y}});}
void sweepTicks(md::GameSession& s,int n=160){for(int i=0;i<n;++i)s.tick();}
bool sweepPress(md::GameSession& s,std::string_view name){
    std::optional<md::Vec2> p;for(const auto& x:s.frame().sprites)if(x.image==name)p=md::Vec2{x.destination.x+x.destination.width/2,x.destination.y+x.destination.height/2};
    if(!p)return false;sweepTap(s,p->x,p->y);sweepTicks(s);return true;
}
void sweepDestroyBase(md::GameSession& s,bool player){
    auto* w=s.validationWorld();if(!w)throw std::runtime_error("Missing sweep world");
    md::ActorId id=0;for(const auto& a:w->actors())if(a->base&&a->player==player)id=a->id;
    if(!id)throw std::runtime_error("Missing sweep base");w->damage(id,w->actor(id)->hp);s.tick();
}
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
// Same strategy as HeavyMach's iOS XCTest suites: production game objects,
// actual graphics/audio backends, isolated persistence, accelerated game ticks.
- (void)testEightyMissionFailureVictoryNextAndDiskReloadSweep {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:CGRectMake(0,0,320,480)];surface.contentScaleFactor=1;surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;
    layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];
    MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds],@"%@",audio.lastError);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    auto initial=md::SaveData::empty(1);initial.createProfile("SWEEP",1);initial.writeWord(md::SaveData::slotStart+0xc,5);
    XCTAssertTrue(host.writeSave(initial.encode()));
    NSMutableArray* records=[NSMutableArray array];const auto start=CFAbsoluteTimeGetCurrent();
    for(unsigned stage=0;stage<80;++stage){@autoreleasepool{
        md::GameSession session(resources,host,true);
        sweepTap(session,160,240);sweepTicks(session);sweepTap(session,160,140);sweepTicks(session);sweepTap(session,240,450);sweepTicks(session);
        auto* world=session.validationWorld();XCTAssertNotEqual(world,nullptr);if(!world)break;
        XCTAssertEqual(world->stage(),stage);
        XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer],@"%@",renderer.lastError);
        auto before=*session.validationSave();const auto writes=host.writes;
        sweepTicks(session,10);XCTAssertEqual(host.writes,writes);
        // Controlled damage exercises result/save/retry, not natural victory.
        sweepDestroyBase(session,true);
        XCTAssertEqual(world->state(),md::WorldState::failed);XCTAssertEqual(host.writes,writes+1);
        auto failed=md::SaveData::decode(*host.readSave());
        XCTAssertEqual(failed.stage(0),stage);XCTAssertEqual(failed.clearCount(0,stage),0u);XCTAssertEqual(failed.credits(0),before.credits(0));
        XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);
        XCTAssertTrue(sweepPress(session,"SummaryBtn_Retry.png"));world=session.validationWorld();
        XCTAssertEqual(world->state(),md::WorldState::running);XCTAssertEqual(world->stage(),stage);
        before=*session.validationSave();sweepDestroyBase(session,false);
        XCTAssertEqual(world->state(),md::WorldState::cleared);
        auto cleared=md::SaveData::decode(*host.readSave());
        XCTAssertEqual(cleared.clearCount(0,stage),1u);XCTAssertEqual(cleared.stage(0),std::min(79u,stage+1));
        XCTAssertEqual(cleared.credits(0),before.credits(0)+500+int(stage)*25);
        XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer],@"%@",renderer.lastError);
        if(stage==0||stage==79)[self capture:[NSString stringWithFormat:@"sweep-stage-%u-result",stage+1] renderer:renderer];
        const auto savedWrites=host.writes;sweepTicks(session,10);XCTAssertEqual(host.writes,savedWrites);
        for(unsigned slot=1;slot<5;++slot)XCTAssertFalse(cleared.active(slot));
        if(stage<79){XCTAssertTrue(sweepPress(session,"SummaryBtn_NextStage.png"));XCTAssertEqual(session.validationWorld()->stage(),stage+1);}
        else{XCTAssertTrue(sweepPress(session,"SummaryBtn_Retry.png"));XCTAssertEqual(session.validationWorld()->stage(),79u);}
        [records addObject:@{@"mission":@(stage+1),@"clearCount":@(cleared.clearCount(0,stage)),@"credits":@(cleared.credits(0)),@"score":@(cleared.readWord(md::SaveData::slotStart+0x2054)),@"writes":@(host.writes)}];
    }}
    XCTAssertEqual(records.count,80u);XCTAssertGreaterThan(audio.playCount,0u);XCTAssertEqual(audio.loadedSounds,80u);
    auto finalSave=md::SaveData::decode(*host.readSave());for(unsigned stage=0;stage<80;++stage)XCTAssertEqual(finalSave.clearCount(0,stage),1u);
    NSDictionary* report=@{@"mode":@"accelerated-result-paths",@"seconds":@(CFAbsoluteTimeGetCurrent()-start),@"soundCalls":@(audio.playCount),@"missions":records};
    NSData* json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:nil];
    XCTAttachment* result=[XCTAttachment attachmentWithData:json uniformTypeIdentifier:@"public.json"];result.name=@"campaign-sweep.json";result.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:result];
    NSLog(@"MD_CAMPAIGN_SWEEP missions=%lu sounds=%lu seconds=%.3f",(unsigned long)records.count,(unsigned long)audio.playCount,CFAbsoluteTimeGetCurrent()-start);
    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;
    XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}

- (void)testPortObservedPurchasesThroughGameButtonsAndDiskSave {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:CGRectMake(0,0,320,480)];surface.contentScaleFactor=1;surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    auto input=md::SaveData::empty(1);input.createProfile("PARITY",1);input.writeWord(md::SaveData::slotStart+0xc,5);input.writeWord(md::SaveData::slotStart+0x2054,274);input.setCredits(0,996100);
    for(unsigned i=0;i<6;++i)input.setBaseUpgrade(0,i,1);
    XCTAssertTrue(host.writeSave(input.encode()));md::GameSession session(resources,host,true);
    sweepTap(session,160,240);sweepTicks(session);sweepTap(session,160,140);sweepTicks(session);sweepTap(session,240,450);sweepTicks(session);
    XCTAssertNotEqual(session.validationWorld(),nullptr);if(!session.validationWorld())return;
    const auto writes=host.writes;
    sweepTap(session,284,469);sweepTicks(session,60);
    const char* baseNames[]={"BaseCannon","BaseDefense","BuildTime","CrResearch","CellResearch","CellStorage"};
    // Expected values captured from the WeMade port, not derived from the implementation.
    const int baseCosts[]={1200,1200,1400,1000,1600,1400};int credits=996100;
    for(unsigned i=0;i<6;++i){
        XCTAssertTrue(sweepPress(session,"Btn_BaseUpgrade_"+std::string(baseNames[i])+".png"));
        XCTAssertTrue(sweepPress(session,"Btn_UpgradeBase_Upgrade.png"));credits-=baseCosts[i];
        auto state=*session.validationSave();XCTAssertEqual(state.credits(0),credits);XCTAssertEqual(state.baseUpgrade(0,i),2u);
        XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);XCTAssertEqual(host.writes,writes);
    }
    XCTAssertEqual(credits,988300);XCTAssertTrue(sweepPress(session,"Btn_UpgradeBase_Close.png"));
    sweepTap(session,294,403);
    const int unitCosts[]={200,400,250,200};
    for(unsigned i=0;i<4;++i){sweepTap(session,32+46*i,70.25f);XCTAssertTrue(sweepPress(session,"UpgradeBtn_Upgrade.png"));credits-=unitCosts[i];
        auto state=*session.validationSave();XCTAssertEqual(state.credits(0),credits);XCTAssertEqual(state.builds(0).at(i).level,1u);
        XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);XCTAssertEqual(host.writes,writes);
    }
    XCTAssertEqual(credits,987250);XCTAssertTrue(sweepPress(session,"Common_Btn_CLOSE.png"));
    sweepTap(session,160,445);sweepTap(session,294,403);
    const int attackCosts[]={600,620,600};
    for(unsigned i=0;i<3;++i){sweepTap(session,60+46*i,70.25f);XCTAssertTrue(sweepPress(session,"UpgradeBtn_Upgrade.png"));credits-=attackCosts[i];
        auto state=*session.validationSave();XCTAssertEqual(state.credits(0),credits);XCTAssertEqual(state.attacks(0).at(i).level,1u);
        XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);XCTAssertEqual(host.writes,writes);
    }
    XCTAssertEqual(credits,985430);XCTAssertTrue(sweepPress(session,"Common_Btn_CLOSE.png"));
    sweepTap(session,284,435);sweepTap(session,130,120);sweepTicks(session);
    XCTAssertEqual(host.writes,writes+1);auto saved=md::SaveData::decode(*host.readSave());
    XCTAssertEqual(saved.credits(0),985430);XCTAssertEqual(saved.readWord(md::SaveData::slotStart+0x2054),274u);
    for(unsigned i=0;i<3;++i)XCTAssertEqual(saved.attacks(0).at(i).count,i==2?1u:2u);
    md::GameSession reloaded(resources,host,true);XCTAssertEqual(reloaded.validationSave()->credits(0),985430);
    for(unsigned i=0;i<6;++i)XCTAssertEqual(reloaded.validationSave()->baseUpgrade(0,i),2u);
    NSLog(@"MD_PORT_PURCHASE_SWEEP base=6 units=4 specials=3 finalCr=985430 score=274 writesDuringPurchases=0");
    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;
    XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}

- (void)testAllProductionAndSpecialLevelsWithGraphicsAndAudio {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:CGRectMake(0,0,320,480)];surface.contentScaleFactor=1;surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    unsigned frames=0,renders=0;const auto start=CFAbsoluteTimeGetCurrent();
    try{
        auto result=md::validation::runContentSweep(resources,host,[&](const md::Frame& frame){
            for(const auto& sprite:frame.sprites){
                auto size=resources.imageSize(sprite.image);XCTAssertGreaterThan(size.x,0.f);XCTAssertGreaterThan(size.y,0.f);
                XCTAssertTrue(std::isfinite(sprite.destination.x)&&std::isfinite(sprite.destination.y));
            }
            // Inspect every generated frame; submit representative GPU frames.
            if(frames++%16==0){XCTAssertTrue([renderer drawFrame:frame toLayer:layer],@"%@",renderer.lastError);++renders;}
        });
        XCTAssertEqual(result.builds,220u);XCTAssertEqual(result.placements,60u);XCTAssertEqual(result.attacks,54u);
        XCTAssertEqual(host.writes,0u);XCTAssertFalse(host.readSave().has_value());XCTAssertGreaterThan(audio.playCount,0u);
        NSLog(@"MD_CONTENT_SWEEP builds=%u placements=%u specials=%u frames=%u renders=%u soundCalls=%lu seconds=%.3f",result.builds,result.placements,result.attacks,frames,renders,(unsigned long)audio.playCount,CFAbsoluteTimeGetCurrent()-start);
    }catch(const std::exception& e){XCTFail(@"Content sweep: %s",e.what());}
    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;
    XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}
@end
