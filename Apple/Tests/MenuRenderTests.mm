#import <XCTest/XCTest.h>
#import <UIKit/UIKit.h>
#import <AVFAudio/AVFAudio.h>
#import <OpenGLES/EAGL.h>
#import "../../Sources/Platform/Apple/MDRenderer.h"
#include "Core/Presentation/Menu.hpp"
#include "Core/Session/GameSession.hpp"
#include "Core/Simulation/World.hpp"
#include "../../Tests/ContentSweep.hpp"
#include "../../Tests/NuclearDamageSweep.hpp"
#include "../../Tests/BaseUpgradeEffectSweep.hpp"
#include "../../Tests/DamageAudioSweep.hpp"
#include "../../Tests/ViewportSweep.hpp"
#import "../../Sources/Platform/Apple/MDAudio.h"
#include <algorithm>
#include <stdexcept>
#include <set>
#include <map>
#include <mach/mach.h>

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
    NSURL* file_; MDAudio* audio_;bool musicOutput_;
public:
    unsigned writes=0;std::map<std::string,unsigned> soundCalls,musicCalls;
    SweepHost(NSURL* file,MDAudio* audio,bool musicOutput=true):file_(file),audio_(audio),musicOutput_(musicOutput){}
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
        ++soundCalls[std::string(n)];
        NSString* name=[[NSString alloc]initWithBytes:n.data() length:n.size() encoding:NSUTF8StringEncoding];
        [audio_ play:name gain:gain*.05f pan:pan];
    }
    void playSoundAt(std::string_view n,md::Vec2 position,float gain)override {
        ++soundCalls[std::string(n)];
        NSString* name=[[NSString alloc]initWithBytes:n.data() length:n.size() encoding:NSUTF8StringEncoding];
        [audio_ play:name gain:gain*.05f x:position.x y:position.y];
    }
    void playMusic(std::string_view n,float gain)override {
        ++musicCalls[std::string(n)];if(!musicOutput_)return;
        NSString* name=[[NSString alloc]initWithBytes:n.data() length:n.size() encoding:NSUTF8StringEncoding];
        [audio_ playMusic:name gain:gain*.05f];
    }
    void stopMusic()override{if(musicOutput_)[audio_ stopMusic];}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)> done)override{done("SWEEP");}
};
void sweepTap(md::GameSession& s,float x,float y){s.touch({1,md::TouchPhase::began,{x,y}});s.touch({1,md::TouchPhase::ended,{x,y}});}
void sweepTicks(md::GameSession& s,int n=160){for(int i=0;i<n;++i)s.tick();}
bool sweepPress(md::GameSession& s,std::string_view name){
    std::optional<md::Vec2> p;for(const auto& x:s.frame().sprites)if(x.image==name)p=md::Vec2{x.destination.x+x.destination.width/2,x.destination.y+x.destination.height/2};
    if(!p)return false;sweepTap(s,p->x,p->y);sweepTicks(s,200);return true;
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
@interface MDSweepDisplay:NSObject
@property(nonatomic,copy) void (^onFrame)(CADisplayLink*);
- (void)frame:(CADisplayLink*)link;
@end
@implementation MDSweepDisplay
- (void)frame:(CADisplayLink*)link {if(self.onFrame)self.onFrame(link);}
@end
@protocol MDSweepHostView
- (instancetype)initWithFrame:(CGRect)frame saveDirectory:(NSURL*)directory;
- (void)setApplicationActive:(BOOL)active;
- (void)audioInterrupted:(NSNotification*)notification;
- (void)audioReset:(NSNotification*)notification;
- (void)shutdown;
@end
@interface MenuRenderTests:XCTestCase @end
@implementation MenuRenderTests
- (void)testExpandedViewportAndInputs {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    UIWindow* window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:window.bounds];surface.contentScaleFactor=1;surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MenuResources resources(root,renderer);
    try {
        auto report=md::validation::runViewportSweep(resources,[&](const md::Frame& f,std::string_view name){
            [renderer discardDrawable];surface.frame=CGRectMake(0,0,std::ceil(f.logicalWidth),std::ceil(f.logicalHeight));[surface layoutIfNeeded];[CATransaction flush];
            XCTAssertTrue([renderer drawFrame:f toLayer:layer],@"%@",renderer.lastError);
            md::Vec2 mapped;XCTAssertTrue([renderer logicalPoint:CGPointMake(f.logicalWidth-1,f.logicalHeight-1) result:&mapped]);
            XCTAssertEqualWithAccuracy(mapped.x,f.logicalWidth-1,1);XCTAssertEqualWithAccuracy(mapped.y,f.logicalHeight-1,1);
            [self capture:[NSString stringWithUTF8String:std::string(name).c_str()] renderer:renderer];
        });
        NSLog(@"MD_VIEWPORT_SWEEP %s",report.c_str());
    }catch(const std::exception& e){XCTFail(@"Viewport: %s",e.what());}
    [renderer finish];window.hidden=YES;
}
- (void)testNuclearDamageAllLevels {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MenuResources resources(root,renderer);
    MenuHost host;
    try {
        const auto report=md::validation::runNuclearDamageSweep(resources,host);
        XCTAttachment* attachment=[XCTAttachment attachmentWithString:[NSString stringWithUTF8String:report.c_str()]];
        attachment.name=@"nuclear-damage-all-levels";attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
        NSLog(@"MD_NUCLEAR_DAMAGE\n%s",report.c_str());XCTAssertEqual(host.saves,0u);
    } catch(const std::exception& e) { XCTFail(@"Nuclear damage: %s",e.what()); }
}
- (void)testBaseUpgradeActualEffects {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MenuResources resources(root,renderer);
    MenuHost host;
    try {
        const auto report=md::validation::runBaseUpgradeEffectSweep(resources,host);
        XCTAttachment* attachment=[XCTAttachment attachmentWithString:[NSString stringWithUTF8String:report.c_str()]];
        attachment.name=@"base-upgrade-actual-effects";attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
        NSLog(@"MD_BASE_EFFECTS\n%s",report.c_str());XCTAssertEqual(host.saves,0u);
    } catch(const std::exception& e) { XCTFail(@"Base upgrade: %s",e.what()); }
}
- (void)testDamageSoundOriginalOrder {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MenuResources resources(root,renderer);
    MenuHost host;
    try {
        const auto report=md::validation::runDamageAudioSweep(resources,host);
        XCTAttachment* attachment=[XCTAttachment attachmentWithString:[NSString stringWithUTF8String:report.c_str()]];
        attachment.name=@"damage-sound-original-order";attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
        NSLog(@"MD_DAMAGE_AUDIO\n%s",report.c_str());XCTAssertEqual(host.saves,0u);
    } catch(const std::exception& e) { XCTFail(@"Damage sound: %s",e.what()); }
}
- (void)setUp {
    [super setUp];
    // On a device XCTest is hosted by the cheat app. Stop its menu renderer;
    // each test below owns an isolated game and temporary save file.
    for(UIWindow* window in UIApplication.sharedApplication.windows){UIView* view=window.rootViewController.view;if([view respondsToSelector:@selector(shutdown)]){[(id<MDSweepHostView>)view shutdown];window.hidden=YES;}}
}
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
- (void)testForegroundRecoversWithoutInterruptionEnded {
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    Class gameClass=NSClassFromString(@"MDGameView");XCTAssertNotNil(gameClass);if(!gameClass)return;
    UIView<MDSweepHostView>* view=[(id<MDSweepHostView>)[gameClass alloc] initWithFrame:CGRectMake(0,0,320,480) saveDirectory:directory];
    UIWindow* window=[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    window.rootViewController.view=view;[window makeKeyAndVisible];[view layoutIfNeeded];
    MDAudio* audio=[view valueForKey:@"audio"];
    [view setApplicationActive:YES];[audio playMusic:@"GamePlay.mp3" gain:.05f];XCTAssertTrue(audio.musicPlaying);
    NSNotification* began=[NSNotification notificationWithName:AVAudioSessionInterruptionNotification object:nil userInfo:@{AVAudioSessionInterruptionTypeKey:@(AVAudioSessionInterruptionTypeBegan)}];
    NSNotification* ended=[NSNotification notificationWithName:AVAudioSessionInterruptionNotification object:nil userInfo:@{AVAudioSessionInterruptionTypeKey:@(AVAudioSessionInterruptionTypeEnded)}];
    for(unsigned cycle=0;cycle<3;++cycle){
        [view audioInterrupted:began];
        // Resume and menu input remain enabled while an audio interruption exists.
        XCTAssertTrue([[view valueForKey:@"active"] boolValue]);XCTAssertFalse(audio.musicPlaying);
        [view setApplicationActive:NO];XCTAssertTrue([[view valueForKey:@"displayLink"] isPaused]);
        [view setApplicationActive:YES]; // Camera may not deliver an interruption-ended notification.
        XCTAssertTrue([[view valueForKey:@"active"] boolValue]);XCTAssertFalse([[view valueForKey:@"displayLink"] isPaused]);
        XCTAssertFalse([[view valueForKey:@"audioInterrupted"] boolValue]);XCTAssertTrue(audio.musicPlaying);
        const auto count=audio.playCount;[audio play:@"BuildMachComplete.wav" gain:.05f pan:0];XCTAssertEqual(audio.playCount,count+1);
        [view audioInterrupted:ended];XCTAssertTrue(audio.musicPlaying);
    }
    // A suspension notification may arrive after the foreground callback.
    // It describes the old background suspension, not a new phone call.
    NSNotification* delayed=[NSNotification notificationWithName:AVAudioSessionInterruptionNotification object:nil userInfo:@{AVAudioSessionInterruptionTypeKey:@(AVAudioSessionInterruptionTypeBegan),AVAudioSessionInterruptionWasSuspendedKey:@YES}];
    [view audioInterrupted:delayed];
    XCTAssertFalse([[view valueForKey:@"audioInterrupted"] boolValue]);
    XCTAssertTrue(audio.musicPlaying);
    audio=nil; // The host owns the audio context; do not retain the retired device in this fixture.
    [view audioReset:[NSNotification notificationWithName:AVAudioSessionMediaServicesWereResetNotification object:nil]];
    audio=[view valueForKey:@"audio"];XCTAssertEqualObjects(audio.musicName,@"GamePlay.mp3");XCTAssertTrue(audio.musicPlaying);
    const float beforePlayback=audio.musicPlaybackTime;
    XCTestExpectation* advances=[self expectationWithDescription:@"AudioQueue playback advances after recovery"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,int64_t(.2*NSEC_PER_SEC)),dispatch_get_main_queue(),^{[advances fulfill];});
    [self waitForExpectations:@[advances] timeout:2];
    XCTAssertGreaterThan(audio.musicPlaybackTime,beforePlayback+.05f);
    [view shutdown];XCTAssertFalse([[view valueForKey:@"active"] boolValue]);XCTAssertFalse(audio.musicPlaying);
    window.hidden=YES;window.rootViewController=nil;view=nil;
    [NSFileManager.defaultManager removeItemAtURL:directory error:nil];
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
- (void)testFeedbackButtonsAndLoadingSound {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    UIWindow* window=[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:CGRectMake(0,0,320,480)];surface.contentScaleFactor=1;surface.layer.contentsScale=1;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MenuResources resources(root,renderer);
    MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    auto save=md::SaveData::empty(123);save.createProfile("FEEDBACK",1);save.writeWord(md::SaveData::slotStart+0xc,5);host.writeSave(save.encode());
    md::GameSession game(resources,host,true);sweepTap(game,160,240);sweepTicks(game);sweepTap(game,160,140);sweepTicks(game);sweepTap(game,240,450);sweepTicks(game);
    XCTAssertNotEqual(game.validationWorld(),nullptr);if(!game.validationWorld())return;
    // GPU pixels, not just the selected tile index, must change on touch-down.
    auto energy=[&](md::Vec2 center){NSData* raw=[renderer readFramebufferRGBA];const auto* b=(const unsigned char*)raw.bytes;const int width=int(renderer.drawablePixelSize.width),height=int(renderer.drawablePixelSize.height);unsigned long total=0;for(int y=int(center.y)-12;y<int(center.y)+12;++y)for(int x=int(center.x)-18;x<int(center.x)+18;++x){const auto i=((height-1-y)*width+x)*4;if(i>=0&&i+2<int(raw.length))total+=b[i]+b[i+1]+b[i+2];}return total;};
    // Choose the other panel first to compare unselected and pressed images.
    for(const md::Vec2 point:{md::Vec2{115.5f,443},md::Vec2{161,443},md::Vec2{208.5f,443}}){
        sweepTap(game,point.x==115.5f?161:115.5f,443);sweepTicks(game,2);
        XCTAssertTrue([renderer drawFrame:game.frame() toLayer:layer]);const auto idle=energy(point);
        game.touch({17,md::TouchPhase::began,point});sweepTicks(game,20);XCTAssertTrue([renderer drawFrame:game.frame() toLayer:layer]);const auto pressed=energy(point);
        XCTAssertGreaterThan(pressed,idle);[self capture:[NSString stringWithFormat:@"hud-pressed-%.0f",point.x] renderer:renderer];
        game.touch({17,md::TouchPhase::cancelled,point});sweepTicks(game,2);
    }
    const auto soundCount=host.soundCalls["Title_TitleIn.wav"];sweepDestroyBase(game,false);
    std::optional<md::Vec2> next;for(const auto& sprite:game.frame().sprites)if(sprite.image=="SummaryBtn_NextStage.png")next=md::Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};
    XCTAssertTrue(next.has_value());if(next)sweepTap(game,next->x,next->y);
    XCTAssertEqual(host.soundCalls["Title_TitleIn.wav"],soundCount+1);sweepTicks(game,85);XCTAssertTrue([renderer drawFrame:game.frame() toLayer:layer]);[self capture:@"feedback-clear-illustration" renderer:renderer];
    sweepTicks(game,120);XCTAssertEqual(host.soundCalls["Title_TitleIn.wav"],soundCount+1);XCTAssertEqual(game.validationWorld()->stage(),1u);
    [audio stopMusic];[audio stopEffects];[audio setActive:NO];[renderer finish];window.hidden=YES;[NSFileManager.defaultManager removeItemAtURL:directory error:nil];
}
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
    // This matrix checks music requests; AudioTests covers the three actual
    // tracks, looping and queue recreation. SFX and GPU remain real here.
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio,false);
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
        if(stage==0){
            std::optional<md::Vec2> retry;for(const auto& sprite:session.frame().sprites)if(sprite.image=="SummaryBtn_Retry.png")retry=md::Vec2{sprite.destination.x+sprite.destination.width/2,sprite.destination.y+sprite.destination.height/2};
            XCTAssertTrue(retry.has_value());if(retry)sweepTap(session,retry->x,retry->y);
            sweepTicks(session,90);XCTAssertTrue(md::validation::has(session.frame(),"StageClear01.png"));
            XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);[self capture:@"original-stage-loading-illustration" renderer:renderer];sweepTicks(session,110);
        }else XCTAssertTrue(sweepPress(session,"SummaryBtn_Retry.png"));world=session.validationWorld();
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
    XCTAssertEqual(records.count,80u);XCTAssertGreaterThan(audio.playCount,0u);XCTAssertEqual(audio.loadedSounds,77u);
    auto finalSave=md::SaveData::decode(*host.readSave());for(unsigned stage=0;stage<80;++stage)XCTAssertEqual(finalSave.clearCount(0,stage),1u);
    NSDictionary* report=@{@"mode":@"accelerated-result-paths",@"seconds":@(CFAbsoluteTimeGetCurrent()-start),@"soundCalls":@(audio.playCount),@"missions":records};
    NSData* json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:nil];
    XCTAttachment* result=[XCTAttachment attachmentWithData:json uniformTypeIdentifier:@"public.json"];result.name=@"campaign-sweep.json";result.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:result];
    NSLog(@"MD_CAMPAIGN_SWEEP missions=%lu sounds=%lu seconds=%.3f",(unsigned long)records.count,(unsigned long)audio.playCount,CFAbsoluteTimeGetCurrent()-start);
    XCTAssertGreaterThan(host.musicCalls["GamePlay.mp3"],0u);XCTAssertGreaterThan(host.musicCalls["UI_BGM.mp3"],0u);
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

- (void)testProductionAvailabilityAndPlacementPrompt {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:window.bounds];surface.contentScaleFactor=UIScreen.mainScreen.scale;surface.layer.contentsScale=surface.contentScaleFactor;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MenuResources resources(root,renderer);MenuHost host;
    auto save=md::SaveData::empty(123);save.createProfile("PLACE",1);save.writeWord(md::SaveData::slotStart+0xc,5);
    const std::vector<md::OwnedBuild> builds{{"RifleTurret",0,true},{"MachineGun",0,true}};save.setBuilds(0,builds);host.saved=save.encode();
    md::GameSession session(resources,host,true);sweepTap(session,160,240);sweepTicks(session);sweepTap(session,160,140);sweepTicks(session);sweepTap(session,240,450);sweepTicks(session);
    auto* world=session.validationWorld();XCTAssertNotEqual(world,nullptr);if(!world)return;
    auto has=[&](std::string_view image){return std::any_of(session.frame().sprites.begin(),session.frame().sprites.end(),[&](const auto& s){return s.image==image;});};
    world->setValidationCells(199);session.tick();XCTAssertTrue(has("BuildMach_Cover.png"));sweepTap(session,292,52);XCTAssertFalse(has("MessageBox.png"));
    XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);[self capture:@"production-insufficient-cells" renderer:renderer];
    world->setValidationCells(500);session.tick();XCTAssertFalse(has("BuildMach_Cover.png"));sweepTap(session,292,52);sweepTicks(session,40);XCTAssertTrue(has("MessageBox.png"));XCTAssertFalse(world->paused());
    XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);[self capture:@"turret-tap-to-build" renderer:renderer];
    sweepTap(session,150,260);XCTAssertEqual(world->productionRows().size(),1u);sweepTicks(session,80);XCTAssertFalse(has("MessageBox.png"));XCTAssertEqual(host.saves,1u);
    XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);[self capture:@"turret-position-accepted" renderer:renderer];
    [renderer finish];window.hidden=YES;
}

- (void)testEveryUpgradeLevelThroughButtonsAndDisk {

    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:window.bounds];surface.contentScaleFactor=UIScreen.mainScreen.scale;surface.layer.contentsScale=surface.contentScaleFactor;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    // This matrix checks music requests; AudioTests covers the three actual
    // tracks, looping and queue recreation. SFX and GPU remain real here.
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio,false);
    auto table=[&](std::string name){auto b=resources.read("Tables/"+name+".tsv");return md::DataTable::parse(std::string_view(reinterpret_cast<const char*>(b.data()),b.size()));};

    const auto started=CFAbsoluteTimeGetCurrent();unsigned purchases=0,blocked=0,reloads=0;
    const char* categories[]={"BaseCannon","BaseDefense","BuildTime","CrResearch","CellResearch","CellStorage"};
    const auto baseTable=table("BaseUpgrade"),parts=table("MachBuild_Parts"),names=table("MachBuild_Name"),special=table("SpAttackList");
    struct Case {int kind;std::string name;unsigned category,level;int cost;bool maximum;};std::vector<Case> cases;
    for(unsigned category=0;category<6;++category){for(unsigned level=0;;++level){char key[80];snprintf(key,sizeof key,"%s_%02u",categories[category],level+1);auto row=baseTable.findRow(key);if(!row)break;char next[80];snprintf(next,sizeof next,"%s_%02u",categories[category],level+2);cases.push_back({0,categories[category],category,level,int(*baseTable.integer(*row,"UpgCost")),!baseTable.findRow(next)});}}
    for(const auto* t:{&parts,&names,&special})for(std::size_t row=0;row<t->rowCount();++row){auto full=std::string(t->value(row,t==&special?"AttackName":"MachName"));auto [name,level]=md::validation::ownedName(full);char suffix[16];snprintf(suffix,sizeof suffix,"_%02u",level+2);auto next=t->findRow(name+suffix);int cost=int(*t->integer(row,"UpgCost"));if(t==&special&&next)cost+=2*(int(*t->integer(*next,"Cost"))-int(*t->integer(row,"Cost")));cases.push_back({t==&special?2:1,name,0,level,cost,!next});}
    for(const auto& item:cases){@autoreleasepool{
        // Test each level at cost-1 and with sufficient money using the real dialog.
        for(bool poor:{true,false}){
            auto seed=md::SaveData::empty(1);seed.createProfile("LEVELS",1);seed.writeWord(md::SaveData::slotStart+0xc,5);
            seed.setCredits(0,poor?std::max(0,item.cost-1):10000000);
            if(item.kind==0)seed.setBaseUpgrade(0,item.category,item.level);
            if(item.kind==1)seed.setBuilds(0,std::vector<md::OwnedBuild>{{item.name,item.level,true}});
            if(item.kind==2)seed.setAttacks(0,std::vector<md::OwnedAttack>{{item.name,item.level,2,true}});
            XCTAssertTrue(host.writeSave(seed.encode()));md::GameSession session(resources,host,true);
            sweepTap(session,160,240);sweepTicks(session);sweepTap(session,160,140);sweepTicks(session);sweepTap(session,240,450);sweepTicks(session);
            if(item.kind==0){sweepTap(session,284,469);sweepTicks(session,60);XCTAssertTrue(sweepPress(session,"Btn_BaseUpgrade_"+item.name+".png"));}
            else {if(item.kind==2)sweepTap(session,160,445);sweepTap(session,294,403);sweepTap(session,item.kind==1?32:60,70.25f);}
            const auto before=*session.validationSave();const auto writes=host.writes;
            // Disabled buttons still receive down/up so their rejection is exercised.
            if(item.kind==0)sweepTap(session,132,456);else sweepTap(session,item.kind==1?198:226,296.75f);
            sweepTicks(session,2);auto after=*session.validationSave();const bool accepted=!poor&&!item.maximum;
            XCTAssertEqual(after.credits(0),before.credits(0)-(accepted?item.cost:0),@"%s level %u poor %d",item.name.c_str(),item.level,poor);
            const unsigned got=item.kind==0?after.baseUpgrade(0,item.category):item.kind==1?after.builds(0)[0].level:after.attacks(0)[0].level;
            XCTAssertEqual(got,item.level+unsigned(accepted),@"%s level %u",item.name.c_str(),item.level);
            XCTAssertEqual(host.writes,writes);
            if(accepted)++purchases;else ++blocked;
            if(item.kind==0&&accepted){auto* w=session.validationWorld();if(item.category==1){for(const auto& a:w->actors())if(a->base&&a->player){char key[80];snprintf(key,sizeof key,"BaseDefense_%02u",got+1);XCTAssertEqual(a->maxHP,int(*baseTable.number(*baseTable.findRow(key),"Param")));}}if(item.category==0){char expected[40];snprintf(expected,sizeof expected,"BaseWpn%02u",got+1);for(const auto& a:w->actors())if(a->base&&a->player)XCTAssertEqual(a->weapons.front().name,std::string(expected));}}
            if(!poor){XCTAssertTrue([renderer drawFrame:session.frame() toLayer:layer]);XCTAssertTrue(sweepPress(session,item.kind==0?"Btn_UpgradeBase_Close.png":"Common_Btn_CLOSE.png"));sweepTap(session,284,435);sweepTap(session,130,120);sweepTicks(session);
                auto disk=md::SaveData::decode(*host.readSave());XCTAssertEqual(disk.credits(0),after.credits(0));md::GameSession restored(resources,host,true);auto saved=*restored.validationSave();
                XCTAssertEqual(item.kind==0?saved.baseUpgrade(0,item.category):item.kind==1?saved.builds(0)[0].level:saved.attacks(0)[0].level,got);++reloads;}
        }
    }}
    XCTAssertGreaterThan(purchases,100u);XCTAssertEqual(reloads,cases.size());
    NSLog(@"MD_UPGRADE_SWEEP rows=%lu purchases=%u rejected=%u diskReloads=%u seconds=%.3f",cases.size(),purchases,blocked,reloads,CFAbsoluteTimeGetCurrent()-started);

    XCTAssertGreaterThan(host.musicCalls["GamePlay.mp3"],0u);XCTAssertGreaterThan(host.musicCalls["UI_BGM.mp3"],0u);
    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;
    XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}

- (void)testEveryWeaponDefinitionFiringAndImpacts {

    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:window.bounds];surface.contentScaleFactor=UIScreen.mainScreen.scale;surface.layer.contentsScale=surface.contentScaleFactor;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    auto table=[&](std::string name){auto b=resources.read("Tables/"+name+".tsv");return md::DataTable::parse(std::string_view(reinterpret_cast<const char*>(b.data()),b.size()));};

    auto weaponsTable=table("WeaponInfo");unsigned conditions=0,shots=0,hits=0;const auto start=CFAbsoluteTimeGetCurrent();
    for(bool phone:{true,false})for(std::size_t row=0;row<weaponsTable.rowCount();++row){@autoreleasepool{
        [audio stopEffects];
        auto weapon=md::WeaponDefinition::fromTable(weaponsTable,row,phone);md::DarwinRandom random(123);md::WeaponController controller;
        md::WeaponFrame input;input.deviceScale=phone?.5f:1;input.target=md::WeaponTarget{{float(weapon.shotRange)*.4f,0},1000000};input.time=100;
        unsigned count=0;std::vector<md::Projectile> bullets;
        auto fire=[&](const md::WeaponFire& event){++count;++shots;XCTAssertTrue(std::isfinite(event.destination.x)&&std::isfinite(event.destination.y));if(event.directDamage){++hits;}else bullets.push_back(md::Projectile::fire(weapon,event.origin,event.destination,event.angle,input.time,random));};
        for(unsigned i=0;i<10000&&count<weapon.shotCount;++i){input.time=100+double(i)/60;auto result=controller.tick(weapon,input,random,fire);if(result.playFireSound)host.playSound(weapon.fireSound,1,0);}
        XCTAssertEqual(count,unsigned(weapon.shotCount),@"%s burst",weapon.name.c_str());
        // The first out-of-range/no-target/status-disabled shot must be rejected.
        for(unsigned mode=0;mode<3;++mode){md::WeaponController blocked;auto f=input;f.time=1000;if(mode==0)f.target->position.x=weapon.shotRange+1;if(mode==1)f.target.reset();if(mode==2)f.statusFlags=2;XCTAssertEqual(blocked.tick(weapon,f,random,[](auto&){}).shots,0u,@"%s gate %u",weapon.name.c_str(),mode);}
        for(auto& bullet:bullets){bool impacted=false;for(unsigned i=0;i<4000&&!impacted&&!bullet.removed;++i){impacted=bullet.tick(weapon);XCTAssertTrue(std::isfinite(bullet.position.x)&&std::isfinite(bullet.position.y));}XCTAssertTrue(impacted||bullet.removed,@"%s projectile timeout",weapon.name.c_str());
            // Exact-center collision tests damage dispatch independently of intentional spread.
            std::vector<md::ProjectileTarget> enemy{{bullet.destination,1000000,20}},player;
            auto impact=md::projectileCollide(weapon,bullet.destination,false,input.time,enemy,player);XCTAssertFalse(impact.empty(),@"%s no centered hit",weapon.name.c_str());hits+=unsigned(impact.size());
        }
        // Stop the accelerated burst before measuring the independent world case.
        [audio stopEffects];
        // Exercise the production world's muzzle, projectile, hit and sound path.
        auto save=md::SaveData::empty(1);save.createProfile("WEAPONS",1);md::World world(resources,host,save,0,phone?md::BattleGeometry::phone():md::BattleGeometry::pad(),123);world.start(0);
        md::WorldActor *source=nullptr,*target=nullptr;for(auto& a:world.actors())if(a->base)(a->player?source:target)=a.get();
        XCTAssertNotEqual(source,nullptr);XCTAssertNotEqual(target,nullptr);if(!source||!target)continue;
        source->weapons.front().name=weapon.name;source->weapons.front().controller.setAngle(1.5707963f);target->weapons.clear();target->position={source->position.x,source->position.y+std::min(40.f,float(weapon.shotRange)*.3f)};
        source->target=target->id;source->targetCheck=10000;source->attackCheck=10000;target->hp=target->maxHP=100000000;source->hp=100000000;
        const auto sounds=audio.playCount;bool active=false;
        for(unsigned i=0;i<unsigned((weapon.reloadDelay+weapon.shotDelay+2)*60);++i){world.tick();world.prepareDraw();active|=target->hp<100000000||!world.projectiles().empty();if(i%60==0||active){md::Frame f;world.appendSprites(f);XCTAssertTrue([renderer drawFrame:f toLayer:layer],@"%s %@",weapon.name.c_str(),renderer.lastError);}if(active)break;}
        XCTAssertTrue(active,@"%s world weapon did not fire",weapon.name.c_str());if(!weapon.fireSound.empty())XCTAssertGreaterThan(audio.playCount,sounds,@"%s sound: %@",weapon.name.c_str(),audio.lastError);++conditions;
    }}
    XCTAssertEqual(conditions,weaponsTable.rowCount()*2);XCTAssertEqual(host.writes,0u);
    NSLog(@"MD_WEAPON_SWEEP definitions=%lu conditions=%u shots=%u hits=%u seconds=%.3f",weaponsTable.rowCount(),conditions,shots,hits,CFAbsoluteTimeGetCurrent()-start);

    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;
    XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}

- (void)testShortRealtimeBattlePerformance {

    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    UIWindow* window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:window.bounds];surface.contentScaleFactor=UIScreen.mainScreen.scale;surface.layer.contentsScale=surface.contentScaleFactor;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    auto table=[&](std::string name){auto b=resources.read("Tables/"+name+".tsv");return md::DataTable::parse(std::string_view(reinterpret_cast<const char*>(b.data()),b.size()));};

    auto save=md::SaveData::empty(1);save.createProfile("PERF",1);md::World world(resources,host,save,0,md::BattleGeometry::phone(),123);world.start(0);
    md::WorldActor *friendly=nullptr,*hostile=nullptr;for(auto& a:world.actors())if(a->base)(a->player?friendly:hostile)=a.get();
    XCTAssertNotEqual(friendly,nullptr);XCTAssertNotEqual(hostile,nullptr);if(!friendly||!hostile)return;
    friendly->hp=friendly->maxHP=100000000;hostile->hp=hostile->maxHP=100000000;
    const auto originalGun=friendly->weapons;friendly->weapons.clear();hostile->weapons.clear();
    struct Sample {unsigned phase;double interval,cpu;NSUInteger sprites;std::uint64_t footprint;};
    std::vector<Sample> samples;samples.reserve(1800);unsigned frames=0;double previous=0;
    auto done=[self expectationWithDescription:@"1800-frame cadence measurement"];
    MDSweepDisplay* observer=[MDSweepDisplay new];
    observer.onFrame=^(CADisplayLink* link){
        // Block below calls a C++ closure to retain mutable test state by reference.
    };
    std::function<void(CADisplayLink*)> run=[&](CADisplayLink* link){
        @autoreleasepool{
        const unsigned phase=frames/450;
        if(frames==450){friendly->weapons=originalGun;hostile->position={friendly->position.x,friendly->position.y+60};friendly->target=hostile->id;}
        if(frames==900){friendly->weapons.front().name="BaseWpn07";}
        if(frames==1350){for(int i=0;i<6;++i){auto id=world.validationSpawn("Tanker_10",i%2==0,{float(i%3)*35-35,friendly->position.y+float(i/3)*100+80});auto* a=world.actor(id);a->hp=a->maxHP=100000000;}}
        md::validation::keepBasesAlive(world);
        const double start=CACurrentMediaTime();world.tick();world.prepareDraw();md::Frame frame;world.appendSprites(frame);
        XCTAssertTrue([renderer drawFrame:frame toLayer:layer],@"%@",renderer.lastError);
        const double elapsed=(CACurrentMediaTime()-start)*1000;
        task_vm_info_data_t info{};mach_msg_type_number_t size=TASK_VM_INFO_COUNT;task_info(mach_task_self(),TASK_VM_INFO,reinterpret_cast<task_info_t>(&info),&size);
        samples.push_back({phase,previous?(link.timestamp-previous)*1000:0,elapsed,frame.sprites.size(),info.phys_footprint});previous=link.timestamp;
        if(++frames==1800){[link invalidate];[done fulfill];}
        }
    };
    observer.onFrame=^(CADisplayLink* link){run(link);};
    CADisplayLink* display=[CADisplayLink displayLinkWithTarget:observer selector:@selector(frame:)];display.preferredFramesPerSecond=60;
    [display addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    [self waitForExpectations:@[done] timeout:120];[display invalidate];observer.onFrame=nil;
    XCTAssertEqual(frames,1800u);XCTAssertEqual(host.writes,0u);XCTAssertGreaterThan(audio.playCount,0u);
    NSMutableArray* report=[NSMutableArray array];
    for(unsigned phase=0;phase<4;++phase){std::vector<double> cadence,cpu;std::uint64_t low=UINT64_MAX,high=0;NSUInteger sprites=0;unsigned over=0;
        for(auto p:samples)if(p.phase==phase&&p.interval>0){cadence.push_back(p.interval);cpu.push_back(p.cpu);if(p.interval>25)++over;low=std::min(low,p.footprint);high=std::max(high,p.footprint);sprites=std::max(sprites,p.sprites);}
        std::sort(cadence.begin(),cadence.end());std::sort(cpu.begin(),cpu.end());if(cadence.empty())continue;
        auto percentile=[](const auto& v,double p){return v[std::min(v.size()-1,std::size_t(p*(v.size()-1)))];};
        NSDictionary* row=@{@"phase":@(phase),@"frames":@(cadence.size()),@"p50_ms":@(percentile(cadence,.5)),@"p95_ms":@(percentile(cadence,.95)),@"p99_ms":@(percentile(cadence,.99)),@"over25ms":@(over),@"cpu_p95_ms":@(percentile(cpu,.95)),@"cpu_max_ms":@(cpu.back()),@"memory_min":@(low),@"memory_max":@(high),@"sprites_max":@(sprites)};
        [report addObject:row];NSLog(@"MD_PERF_PHASE %@",row);
    }
    NSData* json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:nil];XCTAttachment* result=[XCTAttachment attachmentWithData:json uniformTypeIdentifier:@"public.json"];result.name=@"short-realtime-performance.json";result.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:result];
    NSLog(@"MD_PERFORMANCE_SWEEP frames=%u sounds=%lu writes=%u",frames,(unsigned long)audio.playCount,host.writes);

    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;
    XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}
- (void)testTenMinuteBattleLifecycleSoak {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    UIWindow* window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];window.rootViewController=[UIViewController new];
    MDMenuSurface* surface=[[MDMenuSurface alloc]initWithFrame:window.bounds];surface.contentScaleFactor=UIScreen.mainScreen.scale;surface.layer.contentsScale=surface.contentScaleFactor;
    CAEAGLLayer* layer=(CAEAGLLayer*)surface.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@YES,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    [window.rootViewController.view addSubview:surface];[window makeKeyAndVisible];
    MDRenderer* renderer=[[MDRenderer alloc]initWithResourceRoot:root];MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    MenuResources resources(root,renderer);
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    SweepHost host([directory URLByAppendingPathComponent:@"MachDefense.sav"],audio);
    auto save=md::SaveData::empty(1);save.createProfile("SOAK",1);
    md::World world(resources,host,save,0,md::BattleGeometry::phone(),123);
    auto restart=[&]{
        world.start(0);md::WorldActor *friendly=nullptr,*hostile=nullptr;
        for(auto& a:world.actors())if(a->base)(a->player?friendly:hostile)=a.get();
        XCTAssertNotEqual(friendly,nullptr);XCTAssertNotEqual(hostile,nullptr);if(!friendly||!hostile)return;
        friendly->hp=friendly->maxHP=100000000;hostile->hp=hostile->maxHP=100000000;
        hostile->position={friendly->position.x,friendly->position.y+60};friendly->target=hostile->id;
        friendly->weapons.front().name="BaseWpn07";
    };restart();[audio playMusic:@"GamePlay.mp3" gain:.01];
    unsigned frames=0,cycles=0,lastSecond=0;std::vector<std::uint64_t> memory;std::vector<double> cpu;
    const double origin=CACurrentMediaTime();
    auto done=[self expectationWithDescription:@"600 seconds combat, 20 lifecycle entries"];
    MDSweepDisplay* observer=[MDSweepDisplay new];
    std::function<void(CADisplayLink*)> run=[&](CADisplayLink* link){@autoreleasepool{
        const double now=CACurrentMediaTime(),elapsed=now-origin;
        if(unsigned(elapsed/30)>cycles&&cycles<19){
            ++cycles;[audio setActive:NO];[renderer discardDrawable];restart();[audio setActive:YES];
        }
        md::validation::keepBasesAlive(world);world.tick();world.prepareDraw();md::Frame frame;world.appendSprites(frame);
        XCTAssertTrue([renderer drawFrame:frame toLayer:layer]);++frames;
        if(unsigned(elapsed)>lastSecond){
            lastSecond=unsigned(elapsed);task_vm_info_data_t info{};mach_msg_type_number_t count=TASK_VM_INFO_COUNT;
            task_info(mach_task_self(),TASK_VM_INFO,reinterpret_cast<task_info_t>(&info),&count);memory.push_back(info.phys_footprint);cpu.push_back((CACurrentMediaTime()-now)*1000);
            if(lastSecond%60==0)NSLog(@"MD_SOAK second=%u frames=%u memory=%llu uploads=%lu sounds=%lu writes=%u",lastSecond,frames,(unsigned long long)info.phys_footprint,(unsigned long)renderer.textureUploads,(unsigned long)audio.playCount,host.writes);
        }
        if(elapsed>=600){[link invalidate];[done fulfill];}
    }};
    observer.onFrame=^(CADisplayLink* link){run(link);};CADisplayLink* display=[CADisplayLink displayLinkWithTarget:observer selector:@selector(frame:)];display.preferredFramesPerSecond=60;
    [display addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];[self waitForExpectations:@[done] timeout:625];[display invalidate];observer.onFrame=nil;
    XCTAssertEqual(cycles,19u);XCTAssertGreaterThan(frames,15000u);XCTAssertEqual(host.writes,0u);XCTAssertGreaterThan(audio.playCount,100u);XCTAssertTrue(audio.musicPlaying);
    if(memory.size()>120){
        const auto warm=*std::min_element(memory.begin()+60,memory.begin()+120);
        const auto final=*std::min_element(memory.end()-60,memory.end());
        XCTAssertLessThan(final,warm+32ull*1024*1024);
        NSLog(@"MD_SOAK_RESULT frames=%u entries=%u warm_min=%llu final_min=%llu sounds=%lu writes=%u",frames,cycles+1,(unsigned long long)warm,(unsigned long long)final,(unsigned long)audio.playCount,host.writes);
    }
    [audio stopMusic];[audio setActive:NO];[renderer finish];window.hidden=YES;[NSFileManager.defaultManager removeItemAtURL:directory error:nil];
}
@end
