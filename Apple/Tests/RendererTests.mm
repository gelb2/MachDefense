#import <XCTest/XCTest.h>
#import <UIKit/UIKit.h>
#import <ImageIO/ImageIO.h>
#import <OpenGLES/EAGL.h>
#import "../../Sources/Platform/Apple/MDRenderer.h"

@interface MDTestSurface:UIView @end
@implementation MDTestSurface
+ (Class)layerClass{return CAEAGLLayer.class;}
@end
@interface RendererTests:XCTestCase @end
@implementation RendererTests {
    UIWindow* _window;
    MDTestSurface* _surface;
    MDRenderer* _renderer;
    NSURL* _root;
}
- (void)setUp {
    [super setUp];
    _root=[[NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES] URLByAppendingPathComponent:NSUUID.UUID.UUIDString isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:[_root URLByAppendingPathComponent:@"Images"] withIntermediateDirectories:YES attributes:nil error:nil];
    const unsigned char pixels[]={255,0,0,255,0,255,0,255,0,0,255,255,255,255,255,255};
    CGColorSpaceRef color=CGColorSpaceCreateDeviceRGB();CGDataProviderRef provider=CGDataProviderCreateWithData(nullptr,pixels,sizeof(pixels),nullptr);
    CGImageRef image=CGImageCreate(2,2,8,32,8,color,kCGBitmapByteOrder32Big|kCGImageAlphaLast,provider,nullptr,false,kCGRenderingIntentDefault);
    CGImageDestinationRef out=CGImageDestinationCreateWithURL((__bridge CFURLRef)[[_root URLByAppendingPathComponent:@"Images"] URLByAppendingPathComponent:@"four.png"],CFSTR("public.png"),1,nullptr);
    CGImageDestinationAddImage(out,image,nullptr);XCTAssertTrue(CGImageDestinationFinalize(out));CFRelease(out);CGImageRelease(image);CGDataProviderRelease(provider);CGColorSpaceRelease(color);
    _window=[[UIWindow alloc] initWithFrame:CGRectMake(0,0,320,480)];_window.rootViewController=[UIViewController new];
    _surface=[[MDTestSurface alloc] initWithFrame:CGRectMake(0,0,320,480)];_surface.contentScaleFactor=1;_surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)_surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [_window.rootViewController.view addSubview:_surface];[_window makeKeyAndVisible];
    _renderer=[[MDRenderer alloc] initWithResourceRoot:_root];XCTAssertNotNil(_renderer);
}
- (void)tearDown {
    [_renderer finish];_renderer=nil;_window.hidden=YES;_window=nil;_surface=nil;
    [[NSFileManager defaultManager] removeItemAtURL:_root error:nil];[super tearDown];
}
- (void)assertPixel:(NSData*)data x:(NSUInteger)x y:(NSUInteger)y red:(int)r green:(int)g blue:(int)b {
    const auto size=[_renderer drawablePixelSize];XCTAssertGreaterThan(data.length,0u);
    if(!data.length)return;
    const auto* p=(const unsigned char*)data.bytes+(NSUInteger(size.height)-1-y)*NSUInteger(size.width)*4+x*4;
    XCTAssertEqualWithAccuracy(p[0],r,2);XCTAssertEqualWithAccuracy(p[1],g,2);XCTAssertEqualWithAccuracy(p[2],b,2);
}
- (void)testSpriteOrientationCropAndTextureReuse {
    md::Frame f;md::Sprite s;s.image="four.png";s.destination={0,0,320,480};s.nearest=true;s.blend=md::Blend::opaque;f.sprites.push_back(s);
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);
    NSData* data=[_renderer readFramebufferRGBA];
    [self assertPixel:data x:40 y:40 red:255 green:0 blue:0];[self assertPixel:data x:280 y:40 red:0 green:255 blue:0];
    [self assertPixel:data x:40 y:400 red:0 green:0 blue:255];[self assertPixel:data x:280 y:400 red:255 green:255 blue:255];
    XCTAssertEqual(_renderer.textureUploads,1u);
    f.sprites[0].sourcePx=md::Rect{1,0,1,1};
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);
    [self assertPixel:[_renderer readFramebufferRGBA] x:160 y:240 red:0 green:255 blue:0];
    XCTAssertEqual(_renderer.textureUploads,1u);XCTAssertEqual(_renderer.presentedFrames,2u);
}
- (void)testSharedAspectFitAndTouchTransform {
    md::Frame f;f.logicalWidth=320;f.logicalHeight=320;
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);md::Vec2 point;
    XCTAssertFalse([_renderer logicalPoint:CGPointMake(160,40) result:&point]);
    XCTAssertTrue([_renderer logicalPoint:CGPointMake(160,240) result:&point]);XCTAssertEqualWithAccuracy(point.x,160,.01);XCTAssertEqualWithAccuracy(point.y,160,.01);
    XCTAssertTrue([_renderer logicalPoint:CGPointMake(0,80) result:&point]);XCTAssertEqualWithAccuracy(point.x,0,.01);XCTAssertEqualWithAccuracy(point.y,0,.01);
}
- (void)testAlphaAndClip {
    md::Frame f;f.background={0,0,1,1};md::Sprite s;s.destination={0,0,320,480};s.color={1,0,0,.5};s.clip=md::Rect{0,0,160,480};f.sprites.push_back(s);
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);NSData* data=[_renderer readFramebufferRGBA];
    [self assertPixel:data x:40 y:40 red:128 green:0 blue:128];[self assertPixel:data x:280 y:40 red:0 green:0 blue:255];
}
- (void)testDrawableRebuildKeepsTexture {
    md::Frame f;md::Sprite s;s.image="four.png";s.destination={0,0,320,480};s.nearest=true;f.sprites.push_back(s);
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);[_renderer discardDrawable];
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);XCTAssertEqual(_renderer.textureUploads,1u);
    [self assertPixel:[_renderer readFramebufferRGBA] x:40 y:40 red:255 green:0 blue:0];
}
- (void)testImageDimensionsAreCachedBeforeUpload {
    const auto first=[_renderer imageSize:std::string("four.png")];XCTAssertEqual(first.x,2);XCTAssertEqual(first.y,2);
    [[NSFileManager defaultManager] removeItemAtURL:[[_root URLByAppendingPathComponent:@"Images"] URLByAppendingPathComponent:@"four.png"] error:nil];
    const auto cached=[_renderer imageSize:std::string("four.png")];XCTAssertEqual(cached.x,2);XCTAssertEqual(cached.y,2);
    XCTAssertEqual(_renderer.textureUploads,0u);
}
- (void)testPremultipliedOpacityMatchesStraightAlpha {
    md::Frame f;f.background={0,0,1,1};md::Sprite s;s.image="four.png";s.sourcePx=md::Rect{0,0,1,1};s.destination={0,0,320,480};s.color.a=.5;s.nearest=true;f.sprites.push_back(s);
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);NSData* straight=[_renderer readFramebufferRGBA];
    [self assertPixel:straight x:160 y:240 red:128 green:0 blue:128];
    f.sprites[0].blend=md::Blend::premultipliedAlpha;
    XCTAssertTrue([_renderer drawFrame:f toLayer:(CAEAGLLayer*)_surface.layer]);
    [self assertPixel:[_renderer readFramebufferRGBA] x:160 y:240 red:128 green:0 blue:128];
    XCTAssertEqual(_renderer.textureUploads,2u);
}
- (void)testTrailMeshInterleavesWithSpritesAndRestoresColorState {
    md::Frame frame;md::Sprite background;background.destination={0,0,320,480};background.color={0,1,0,1};frame.sprites.push_back(background);
    md::TextureMesh mesh;mesh.image="four.png";mesh.beforeSpriteIndex=1;mesh.blend=md::Blend::opaque;
    mesh.vertices={{{0,0},{.25f,.25f},{1,1,1,1}},{{320,0},{.25f,.25f},{1,1,1,1}},{{0,480},{.25f,.25f},{1,1,1,1}},{{320,480},{.25f,.25f},{1,1,1,1}}};frame.meshes.push_back(mesh);
    md::Sprite foreground;foreground.destination={160,0,160,480};foreground.color={0,0,1,1};frame.sprites.push_back(foreground);
    XCTAssertTrue([_renderer drawFrame:frame toLayer:(CAEAGLLayer*)_surface.layer]);NSData* data=[_renderer readFramebufferRGBA];
    [self assertPixel:data x:40 y:40 red:255 green:0 blue:0];[self assertPixel:data x:280 y:40 red:0 green:0 blue:255];
    XCTAssertEqual([_renderer lastFrameMetrics].drawCalls,3u);
}
@end
