#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import "MDRenderer.h"
#import "MDAudio.h"
#include "Core/Contracts/Game.hpp"
#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>
#include <cstdio>

#ifndef MD_VALIDATION
#define MD_VALIDATION 0
#endif
@class MDGameView;
namespace {
dispatch_queue_t audioSessionQueue(){
    static dispatch_queue_t queue;static dispatch_once_t once;
    dispatch_once(&once,^{queue=dispatch_queue_create("com.gelb2.machdefense.audio-session",DISPATCH_QUEUE_SERIAL);});
    return queue;
}
NSURL* resourceRoot(){
    NSURL* url=[NSBundle.mainBundle URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    NSBundle* bundle=url?[NSBundle bundleWithURL:url]:nil;
    return bundle.resourceURL;
}
class Resources final:public md::ResourceAccess {
    NSURL* __strong root_;MDRenderer* __strong renderer_;
public:
    Resources(NSURL* root,MDRenderer* renderer):root_(root),renderer_(renderer){}
    md::Bytes read(std::string_view path) override {
        if(path.empty()||path.front()=='/'||path.find("..")!=std::string_view::npos)throw std::runtime_error("Invalid resource path");
        NSString* p=[[NSString alloc] initWithBytes:path.data() length:path.size() encoding:NSUTF8StringEncoding];
        NSData* data=[NSData dataWithContentsOfURL:[root_ URLByAppendingPathComponent:p]];
        if(!data)throw std::runtime_error("Missing prepared resource: "+std::string(path));
        const auto* b=static_cast<const std::uint8_t*>(data.bytes);return {b,b+data.length};
    }
    md::Vec2 imageSize(std::string_view name) override{return [renderer_ imageSize:std::string(name)];}
};
}
@interface MDGameView:UIView
- (instancetype)initWithFrame:(CGRect)frame saveDirectory:(NSURL*)directory;
- (void)setApplicationActive:(BOOL)active;
- (void)shutdown;
- (void)requestName:(std::string)initial maximum:(std::size_t)maximum completion:(std::function<void(std::optional<std::string>)>)completion;
@end
namespace {
class Services final:public md::HostServices {
    MDGameView* __weak view_;MDAudio* __strong audio_;NSURL* __strong saveURL_;
    std::string music_;float musicGain_=1;
public:
    bool audit=false;unsigned auditSounds=0,auditWrites=0;double auditAudioMs=0;
private:
public:
    Services(MDGameView* view,MDAudio* audio,NSURL* directory):view_(view),audio_(audio){
        NSURL* root=directory;
        if(!root){root=[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask].firstObject;
            root=[root URLByAppendingPathComponent:@"MachDefense" isDirectory:YES];}
        NSError* error=nil;if(![[NSFileManager defaultManager] createDirectoryAtURL:root withIntermediateDirectories:YES attributes:nil error:&error])throw std::runtime_error("Cannot create save directory");
        saveURL_=[root URLByAppendingPathComponent:@"MachDefense.sav"];
    }
    std::optional<md::Bytes> readSave() override {
        NSError* error=nil;NSData* data=[NSData dataWithContentsOfURL:saveURL_ options:0 error:&error];
        if(!data){if(error.code==NSFileReadNoSuchFileError)return {};throw std::runtime_error("Cannot read save file");}
        const auto* b=static_cast<const std::uint8_t*>(data.bytes);return md::Bytes(b,b+data.length);
    }
    bool writeSave(std::span<const std::uint8_t> bytes) override {
        if(bytes.empty())return false;
        if(audit)++auditWrites;
        NSError* error=nil;BOOL ok=[[NSData dataWithBytes:bytes.data() length:bytes.size()] writeToURL:saveURL_ options:NSDataWritingAtomic error:&error];
        if(!ok)NSLog(@"MachDefense save failed: %@",error);return ok;
    }
    void playSound(std::string_view name,float gain,float pan) override {
        const double start=audit?CACurrentMediaTime():0;
        NSString* n=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];[audio_ play:n gain:gain pan:pan];
        if(audit){++auditSounds;auditAudioMs+=(CACurrentMediaTime()-start)*1000;}
    }
    void playSoundAt(std::string_view name,md::Vec2 position,float gain) override {
        const double start=audit?CACurrentMediaTime():0;
        NSString* n=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
        [audio_ play:n gain:gain x:position.x y:position.y];
        if(audit){++auditSounds;auditAudioMs+=(CACurrentMediaTime()-start)*1000;}
    }
    void playMusic(std::string_view name,float gain) override {
        music_=name;musicGain_=gain;
        NSString* n=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];[audio_ playMusic:n gain:gain];
    }
    void stopMusic() override {music_.clear();[audio_ stopMusic];}
    void requestName(std::string initial,std::size_t maximum,std::function<void(std::optional<std::string>)> completion) override {
        [view_ requestName:std::move(initial) maximum:maximum completion:std::move(completion)];
    }
    void setAudio(MDAudio* audio){audio_=audio;if(audio_&&!music_.empty())playMusic(music_,musicGain_);}
};
}
@implementation MDGameView {
    MDRenderer* _renderer;
    MDAudio* _audio;
    CADisplayLink* _displayLink;
    std::unique_ptr<Resources> _resources;
    std::unique_ptr<Services> _services;
    std::unique_ptr<md::Game> _game;
    std::map<UITouch*,md::Touch> _touches;
    std::uint64_t _nextTouch;
    std::uint64_t _nameRequestGeneration;
    BOOL _active,_sceneActive,_audioInterrupted,_nameInputPending,_shutdown;
    BOOL _audioSessionReady,_audioSessionPending;
    std::uint64_t _audioSessionGeneration;
    struct FrameSample {double time,interval,callbackInterval,tick,submit,present,audio;NSUInteger sprites,uploads;unsigned sounds,writes;};
    std::vector<FrameSample> _samples;
    BOOL _auditFrames;double _auditOrigin,_previousFrameTimestamp,_previousCallback;

}
+ (Class)layerClass{return CAEAGLLayer.class;}
- (instancetype)initWithFrame:(CGRect)frame {
    return [self initWithFrame:frame saveDirectory:nil];
}
- (instancetype)initWithFrame:(CGRect)frame saveDirectory:(NSURL*)directory {
    if(!(self=[super initWithFrame:frame]))return nil;
    self.multipleTouchEnabled=YES;self.opaque=YES;self.backgroundColor=UIColor.blackColor;
    CAEAGLLayer* layer=(CAEAGLLayer*)self.layer;layer.opaque=YES;layer.drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@NO,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGBA8};
    NSURL* root=resourceRoot();if(!root)throw std::runtime_error("MachDefenseAssets.bundle is missing");
    _renderer=[[MDRenderer alloc] initWithResourceRoot:root];if(!_renderer)throw std::runtime_error("OpenGL ES initialization failed");
    _audio=[[MDAudio alloc] initWithResourceRoot:root];[_audio prepareSounds];[_audio setActive:NO];
    _resources=std::make_unique<Resources>(root,_renderer);_services=std::make_unique<Services>(self,_audio,directory);
    _auditFrames=MD_VALIDATION&&[NSProcessInfo.processInfo.environment[@"MD_FRAME_AUDIT"] isEqualToString:@"1"];
    if(_auditFrames){_samples.reserve(18000);_services->audit=true;}
    _game=md::createGame(*_resources,*_services,MD_VALIDATION!=0);
    if(!_game)throw std::runtime_error("Game initialization failed");
    _displayLink=[CADisplayLink displayLinkWithTarget:self selector:@selector(display:)];
    _displayLink.preferredFramesPerSecond=std::clamp(_game->preferredFramesPerSecond(),1,120);_displayLink.paused=YES;
    [_displayLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(audioInterrupted:) name:AVAudioSessionInterruptionNotification object:nil];
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(audioReset:) name:AVAudioSessionMediaServicesWereResetNotification object:nil];
    return self;
}
- (void)dealloc {[_displayLink invalidate];[NSNotificationCenter.defaultCenter removeObserver:self];}
- (void)shutdown {
    if(_shutdown)return;_shutdown=YES;++_nameRequestGeneration;_nameInputPending=NO;
    [self setApplicationActive:NO];[_displayLink invalidate];_displayLink=nil;
    [self.window.rootViewController dismissViewControllerAnimated:NO completion:nil];
    [NSNotificationCenter.defaultCenter removeObserver:self];
}
- (void)layoutSubviews {
    [super layoutSubviews];self.contentScaleFactor=self.window.screen.scale?:UIScreen.mainScreen.scale;self.layer.contentsScale=self.contentScaleFactor;
    if(_game){_game->resize(float(self.bounds.size.width),float(self.bounds.size.height),float(self.safeAreaInsets.top),float(self.safeAreaInsets.bottom));if(_active)[_renderer drawFrame:_game->frame() toLayer:(CAEAGLLayer*)self.layer];}
}
- (void)display:(CADisplayLink*)link {
    if(!_active||_nameInputPending||!_game)return;
    if(!_auditFrames){_game->tick();[_renderer drawFrame:_game->frame() toLayer:(CAEAGLLayer*)self.layer];return;}
    const double start=CACurrentMediaTime();if(!_auditOrigin)_auditOrigin=start;
    _services->auditSounds=0;_services->auditWrites=0;_services->auditAudioMs=0;
    const NSUInteger uploads=_renderer.textureUploads;
    _game->tick();const double ticked=CACurrentMediaTime();
    [_renderer drawFrame:_game->frame() toLayer:(CAEAGLLayer*)self.layer];
    const auto m=[_renderer lastFrameMetrics];
    if(_samples.size()<18000)_samples.push_back({(start-_auditOrigin)*1000,
        _previousFrameTimestamp?(link.timestamp-_previousFrameTimestamp)*1000:0,
        _previousCallback?(start-_previousCallback)*1000:0,(ticked-start)*1000,
        m.submitMilliseconds,m.presentMilliseconds,_services->auditAudioMs,m.sprites,
        _renderer.textureUploads-uploads,_services->auditSounds,_services->auditWrites});
    _previousFrameTimestamp=link.timestamp;_previousCallback=start;
}
- (void)writeFrameAudit {
    if(!_auditFrames||_samples.empty())return;
    // Explicit diagnostic export on inactivity only, never during combat frames.
    NSURL* directory=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    NSURL* url=[directory URLByAppendingPathComponent:@"frame-audit.csv"];
    FILE* file=fopen(url.fileSystemRepresentation,"w");if(!file)return;
    fprintf(file,"time_ms,display_interval_ms,callback_interval_ms,tick_ms,submit_ms,present_ms,audio_ms,sprites,uploads,sounds,writes\n");
    for(const auto& p:_samples)fprintf(file,"%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%lu,%lu,%u,%u\n",p.time,p.interval,p.callbackInterval,p.tick,p.submit,p.present,p.audio,(unsigned long)p.sprites,(unsigned long)p.uploads,p.sounds,p.writes);
    fclose(file);_samples.clear();_auditOrigin=0;
}
- (void)cancelTouches {
    auto pending=std::move(_touches);_touches.clear();
    if(_game)for(const auto& [native,event]:pending){auto cancelled=event;cancelled.phase=md::TouchPhase::cancelled;_game->touch(cancelled);}
}
// Session negotiation can block on the audio daemon. Serialize only OS session
// calls here; OpenAL, AudioQueue ownership and game state stay on the main thread.
- (void)requestAudioSession:(BOOL)active {
    const auto generation=++_audioSessionGeneration;
    _audioSessionReady=NO;_audioSessionPending=YES;
    __weak MDGameView* weakSelf=self;
    dispatch_async(audioSessionQueue(),^{
        NSCAssert(!NSThread.isMainThread,@"Audio session negotiation must not block game input");
        NSError* error=nil;AVAudioSession* session=AVAudioSession.sharedInstance;
        BOOL ok=YES;
        if(active)ok=[session setCategory:AVAudioSessionCategoryAmbient error:&error];
        if(ok)ok=[session setActive:active withOptions:active?0:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error:&error];
        if(!ok)NSLog(@"MachDefense audio session active=%d: %@",active,error);
        dispatch_async(dispatch_get_main_queue(),^{
            MDGameView* view=weakSelf;if(!view||generation!=view->_audioSessionGeneration)return;
            view->_audioSessionPending=NO;
            view->_audioSessionReady=ok&&active&&view->_sceneActive&&!view->_shutdown;
            if(view->_audioSessionReady)view->_audioInterrupted=NO;
            [view updateActivity];
        });
    });
}
- (void)setApplicationActive:(BOOL)active {
    _sceneActive=active;
    _audioSessionReady=NO;
    [self updateActivity]; // Pause players before requesting session deactivation.
    [self requestAudioSession:active&&!_shutdown];
}
- (void)updateActivity {
    const BOOL active=_sceneActive&&!_shutdown;
    _displayLink.paused=!active||_nameInputPending;
    // An audio interruption must not disable Resume or other game input.
    [_audio setActive:active&&_audioSessionReady&&!_audioInterrupted];
    if(_active==active)return;_active=active;
    if(!active){[self cancelTouches];[_renderer finish];[_renderer discardDrawable];[self writeFrameAudit];_previousFrameTimestamp=0;_previousCallback=0;}
    if(_game)_game->setActive(active);
}
- (void)audioInterrupted:(NSNotification*)notification {
    if(!NSThread.isMainThread){__weak MDGameView* weakSelf=self;dispatch_async(dispatch_get_main_queue(),^{[weakSelf audioInterrupted:notification];});return;}
    if(_shutdown)return;
    NSNumber* kind=notification.userInfo[AVAudioSessionInterruptionTypeKey];
    _audioInterrupted=kind.unsignedIntegerValue==AVAudioSessionInterruptionTypeBegan;
    const BOOL wasSuspended=[notification.userInfo[AVAudioSessionInterruptionWasSuspendedKey] boolValue];
    // Invalidate an in-flight activation when a real interruption begins.
    ++_audioSessionGeneration;_audioSessionPending=NO;_audioSessionReady=NO;
    if(_audioInterrupted&&_game){_game->setActive(false);_game->setActive(_sceneActive&&!_shutdown);}
    [self updateActivity];
    if(_sceneActive&&(!_audioInterrupted||wasSuspended))[self requestAudioSession:YES];
    if(MD_VALIDATION)NSLog(@"MD_AUDIO interruption=%@ suspended=%d scene=%d blocked=%d",kind,wasSuspended,_sceneActive,_audioInterrupted);
    [self updateActivity];
}
- (void)audioReset:(NSNotification*)notification {
    if(!NSThread.isMainThread){__weak MDGameView* weakSelf=self;dispatch_async(dispatch_get_main_queue(),^{[weakSelf audioReset:notification];});return;}
    if(_shutdown)return;
    NSString* music=[_audio.musicName copy];const float gain=_audio.musicGain;
    ++_audioSessionGeneration;_audioSessionPending=NO;_audioSessionReady=NO;
    [_audio setActive:NO];_services->setAudio(nil);_audio=nil;
    _audio=[[MDAudio alloc] initWithResourceRoot:resourceRoot()];[_audio prepareSounds];
    [_audio setActive:NO];_services->setAudio(_audio);
    if(music.length)[_audio playMusic:music gain:gain];
    [self requestAudioSession:_sceneActive&&!_shutdown];
#if MD_VALIDATION
    NSLog(@"MD_AUDIO reset captured=%@ restored=%@ buffers=%lu error=%@",music,_audio.musicName,(unsigned long)_audio.loadedSounds,_audio.lastError);
#endif
}
- (void)sendTouches:(NSSet<UITouch*>*)touches phase:(md::TouchPhase)phase {
    if(!_active||_nameInputPending||!_game)return;
    for(UITouch* native in touches){
        auto found=_touches.find(native);md::Vec2 point;
        const BOOL inside=[_renderer logicalPoint:[native locationInView:self] result:&point];
        if(phase==md::TouchPhase::began){if(!inside)continue;md::Touch t{++_nextTouch,phase,point};_touches[native]=t;_game->touch(t);}
        else if(found!=_touches.end()){
            auto t=found->second;t.phase=inside?phase:md::TouchPhase::cancelled;if(inside)t.position=point;
            // Core callbacks may open the name dialog and cancel all touches.
            // Update the registry before dispatch so no iterator survives them.
            if(t.phase==md::TouchPhase::ended||t.phase==md::TouchPhase::cancelled)_touches.erase(found);else found->second=t;
            _game->touch(t);
        }
    }
}
- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {[self sendTouches:touches phase:md::TouchPhase::began];}
- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {[self sendTouches:touches phase:md::TouchPhase::moved];}
- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {[self sendTouches:touches phase:md::TouchPhase::ended];}
- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {[self sendTouches:touches phase:md::TouchPhase::cancelled];}
- (void)requestName:(std::string)initial maximum:(std::size_t)maximum completion:(std::function<void(std::optional<std::string>)>)completion {
    UIViewController* controller=self.window.rootViewController;
    if(_shutdown)return;
    if(!controller||controller.presentedViewController||_nameInputPending){completion({});return;}
    [self cancelTouches];_nameInputPending=YES;[self updateActivity];
    const auto generation=++_nameRequestGeneration;
    __weak MDGameView* weakSelf=self;
    auto finish=[weakSelf,generation,completion](std::optional<std::string> name){
        MDGameView* strongSelf=weakSelf;
        if(!strongSelf||strongSelf->_shutdown||strongSelf->_nameRequestGeneration!=generation)return;
        ++strongSelf->_nameRequestGeneration;strongSelf->_nameInputPending=NO;
        completion(std::move(name));[strongSelf updateActivity];
    };
    UIAlertController* alert=[UIAlertController alertControllerWithTitle:@"Profile name" message:nil preferredStyle:UIAlertControllerStyleAlert];
    [alert addTextFieldWithConfigurationHandler:^(UITextField* field){field.text=[NSString stringWithUTF8String:initial.c_str()];field.autocorrectionType=UITextAutocorrectionTypeNo;field.spellCheckingType=UITextSpellCheckingTypeNo;}];
    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:^(UIAlertAction* action){finish({});}]];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction* action){
        NSString* name=alert.textFields.firstObject.text?:@"";
        while([name lengthOfBytesUsingEncoding:NSUTF8StringEncoding]>maximum&&name.length){NSRange last=[name rangeOfComposedCharacterSequenceAtIndex:name.length-1];name=[name substringToIndex:last.location];}
        finish(std::string(name.UTF8String?:""));
    }]];
    [controller presentViewController:alert animated:YES completion:nil];
}
@end
@interface MDViewController:UIViewController @end
@implementation MDViewController
- (void)loadView {
#if MD_VALIDATION
    // XCTest supplies its own renderer, audio and isolated save. Do not open a
    // second OpenAL device or the player's save while hosting those tests.
    if(NSClassFromString(@"XCTestCase")||NSProcessInfo.processInfo.environment[@"XCTestConfigurationFilePath"]){self.view=[UIView new];return;}
#endif
    self.view=[[MDGameView alloc] initWithFrame:UIScreen.mainScreen.bounds];
}
- (BOOL)prefersStatusBarHidden{return YES;}
- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    return UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPad
        ?(UIInterfaceOrientationMaskPortrait|UIInterfaceOrientationMaskPortraitUpsideDown)
        :UIInterfaceOrientationMaskPortrait;
}
- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures{return UIRectEdgeAll;}
@end
@interface MDSceneDelegate:UIResponder<UIWindowSceneDelegate>
@property(nonatomic,strong) UIWindow* window;
@end
@implementation MDSceneDelegate
- (void)scene:(UIScene*)scene willConnectToSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
    if(![scene isKindOfClass:UIWindowScene.class])return;
    self.window=[[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];self.window.rootViewController=[MDViewController new];[self.window makeKeyAndVisible];
}
- (void)sceneDidBecomeActive:(UIScene*)scene {if([self.window.rootViewController.view respondsToSelector:@selector(setApplicationActive:)])[(MDGameView*)self.window.rootViewController.view setApplicationActive:YES];}
- (void)sceneWillResignActive:(UIScene*)scene {if([self.window.rootViewController.view respondsToSelector:@selector(setApplicationActive:)])[(MDGameView*)self.window.rootViewController.view setApplicationActive:NO];}
- (void)sceneDidDisconnect:(UIScene*)scene {if([self.window.rootViewController.view respondsToSelector:@selector(shutdown)])[(MDGameView*)self.window.rootViewController.view shutdown];self.window=nil;}
@end
@interface MDAppDelegate:UIResponder<UIApplicationDelegate> @end
@implementation MDAppDelegate
- (UISceneConfiguration*)application:(UIApplication*)application configurationForConnectingSceneSession:(UISceneSession*)session options:(UISceneConnectionOptions*)options {
    UISceneConfiguration* config=[[UISceneConfiguration alloc] initWithName:@"Game" sessionRole:session.role];config.delegateClass=MDSceneDelegate.class;return config;
}
@end
int main(int argc,char* argv[]){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(MDAppDelegate.class));}}
