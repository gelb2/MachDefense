#import "MDAudio.h"
#include "MDAudioQueueMusic.hpp"
#import <AudioToolbox/AudioToolbox.h>
#include "Audio/OpenALEffects.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace {
bool decode(NSURL* url,md::audio::PCM& output){
    // Original effect loader 0x49a20 reads PCM bytes without resampling or
    // converting 8-bit/stereo resources into a different sample format.
    AudioFileID file=nullptr;if(AudioFileOpenURL((__bridge CFURLRef)url,kAudioFileReadPermission,0,&file))return false;
    struct Close {AudioFileID f;~Close(){AudioFileClose(f);}} close{file};
    AudioStreamBasicDescription pcm{};UInt32 size=sizeof(pcm);
    if(AudioFileGetProperty(file,kAudioFilePropertyDataFormat,&size,&pcm))return false;
    if(pcm.mFormatID!=kAudioFormatLinearPCM||pcm.mChannelsPerFrame<1||pcm.mChannelsPerFrame>2||
       (pcm.mBitsPerChannel!=8&&pcm.mBitsPerChannel!=16)||!std::isfinite(pcm.mSampleRate)||
       pcm.mSampleRate<1||pcm.mSampleRate>192000||(pcm.mFormatFlags&kAudioFormatFlagIsBigEndian))return false;
    UInt64 bytes=0;size=sizeof(bytes);
    if(AudioFileGetProperty(file,kAudioFilePropertyAudioDataByteCount,&size,&bytes)||!bytes||bytes>64*1024*1024)return false;
    auto& samples=output.bytes;
    samples.resize(bytes);UInt32 count=UInt32(bytes);
    if(AudioFileReadBytes(file,false,0,&count,samples.data())||count!=bytes)return false;
    output.sampleRate=unsigned(pcm.mSampleRate);output.channels=pcm.mChannelsPerFrame;output.bitsPerSample=pcm.mBitsPerChannel;
    return true;
}
}
@implementation MDAudio {
    NSURL* _root;
    md::audio::OpenALEffects _effects;
    BOOL _active;

    md::AudioQueueMusic _music;
    NSString* _musicName;
}
- (instancetype)initWithResourceRoot:(NSURL*)root {
    if(!(self=[super init]))return nil;_root=root;_active=YES;
    if(!_effects.valid())_lastError=[NSString stringWithUTF8String:_effects.error().c_str()];
    return self;
}
- (void)dealloc {_music.close();}
- (BOOL)prepareSounds {
    if(!_effects.valid())return NO;
    NSURL* directory=[_root URLByAppendingPathComponent:@"Audio"];
    NSArray<NSURL*>* files=[[NSFileManager defaultManager] contentsOfDirectoryAtURL:directory includingPropertiesForKeys:nil options:0 error:nil];
    BOOL ok=files!=nil;
    for(NSURL* url in files){
        NSString* ext=url.pathExtension.lowercaseString;if(![ext isEqualToString:@"wav"])continue;
        std::string name=url.lastPathComponent.UTF8String;if(_effects.contains(name))continue;
        md::audio::PCM pcm;
        if(!decode(url,pcm)){_lastError=[NSString stringWithFormat:@"Cannot decode %@",url.lastPathComponent];ok=NO;continue;}
        if(!_effects.prepare(std::move(name),pcm)){_lastError=[NSString stringWithUTF8String:_effects.error().c_str()];ok=NO;}
    }
    if(!ok)NSLog(@"MachDefense audio: %@",_lastError);return ok;
}
- (void)play:(NSString*)name gain:(float)gain pan:(float)pan {
    if(!name.length||!std::isfinite(pan))return;
    _effects.play(name.UTF8String,gain,std::clamp(pan,-1.f,1.f),0);
    if(!_effects.error().empty())_lastError=[NSString stringWithUTF8String:_effects.error().c_str()];
}
- (void)play:(NSString*)name gain:(float)gain x:(float)x y:(float)y {
    if(!name.length)return;_effects.play(name.UTF8String,gain,x,y);
    if(!_effects.error().empty())_lastError=[NSString stringWithUTF8String:_effects.error().c_str()];
}
- (float)effectPlaybackTime:(NSString*)name {return name.length?_effects.playbackTime(name.UTF8String):-1;}
- (void)stopEffects {_effects.stop();}
- (void)setActive:(BOOL)active {
    _active=active;
    if(active){_effects.setActive(true);if(_musicName.length&&!_music.playing())_music.start();}
    else{_music.pause();_effects.setActive(false);}
}
- (void)playMusic:(NSString*)name gain:(float)gain {
    if(!name.length||!std::isfinite(gain))return;
    if(![_musicName isEqualToString:name]){
        NSURL* url=[[_root URLByAppendingPathComponent:@"Audio"] URLByAppendingPathComponent:name];
        if(!_music.open((__bridge CFURLRef)url)){
            _lastError=[NSString stringWithFormat:@"AudioQueue cannot open %@ (%d)",name,int(_music.error())];
            _music.close();_musicName=nil;return;
        }
        _musicName=[name copy];
    }
    _music.setGain(std::clamp(gain,0.f,1.f));
    if(_active&&!_music.playing()&&!_music.start())_lastError=[NSString stringWithFormat:@"AudioQueue start failed (%d)",int(_music.error())];
}
- (void)stopMusic {_music.close();_musicName=nil;}
- (NSString*)musicName{return _musicName;}
- (BOOL)musicPlaying {return _music.playing();}
- (BOOL)musicLooping {return _musicName.length>0;}
- (float)musicGain {return _music.gain();}
- (float)musicPlaybackTime {return _music.time();}
- (NSUInteger)currentContextIdentity{return _effects.currentContextIdentity();}
- (NSUInteger)playCount{return _effects.playCount();}
- (NSUInteger)loadedSounds{return _effects.loaded();}
@end
