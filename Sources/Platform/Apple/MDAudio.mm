#import "MDAudio.h"
#include "MDAudioQueueMusic.hpp"
#import <AudioToolbox/AudioToolbox.h>
#import <AL/al.h>
#import <AL/alc.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace {
struct Sound {ALuint buffer=0,source=0;};
struct Scope {
    ALCcontext* previous=alcGetCurrentContext();ALCcontext* current=nullptr;bool valid=false;
    explicit Scope(ALCcontext* c){valid=c&&(previous==c||alcMakeContextCurrent(c));current=c;}
    ~Scope(){if(valid&&previous!=current)alcMakeContextCurrent(previous);}
};
bool decode(NSURL* url,std::vector<std::uint8_t>& samples,ALsizei& rate,ALenum& format){
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
    samples.resize(bytes);UInt32 count=UInt32(bytes);
    if(AudioFileReadBytes(file,false,0,&count,samples.data())||count!=bytes)return false;
    rate=ALsizei(pcm.mSampleRate);
    format=pcm.mBitsPerChannel==8?(pcm.mChannelsPerFrame==1?AL_FORMAT_MONO8:AL_FORMAT_STEREO8):
                                   (pcm.mChannelsPerFrame==1?AL_FORMAT_MONO16:AL_FORMAT_STEREO16);
    return true;
}
}
@implementation MDAudio {
    NSURL* _root;
    ALCdevice* _device;
    ALCcontext* _context;
    BOOL _active;
    std::map<std::string,Sound> _sounds;
    md::AudioQueueMusic _music;
    NSString* _musicName;
}
- (instancetype)initWithResourceRoot:(NSURL*)root {
    if(!(self=[super init]))return nil;_root=root;_active=YES;
    _device=alcOpenDevice(nullptr);if(_device)_context=alcCreateContext(_device,nullptr);
    // Original 0x48ddc keeps its context current after initialization. Detaching
    // it after each sound stops RemoteIO on iPad and stalls the game thread.
    if(!_context||!alcMakeContextCurrent(_context)){_lastError=@"Cannot initialize OpenAL";return self;}
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);alListener3f(AL_POSITION,0,0,0);alListenerf(AL_GAIN,1);
    return self;
}
- (void)dealloc {
    _music.close();
    ALCcontext* previous=alcGetCurrentContext();if(_context&&alcMakeContextCurrent(_context)){
        for(auto& [name,s]:_sounds){alSourceStop(s.source);alDeleteSources(1,&s.source);alDeleteBuffers(1,&s.buffer);}
    }
    if(_context){alcMakeContextCurrent(previous==_context?nullptr:previous);alcDestroyContext(_context);}
    if(_device)alcCloseDevice(_device);
}
- (BOOL)prepareSounds {
    Scope scope(_context);if(!scope.valid)return NO;
    NSURL* directory=[_root URLByAppendingPathComponent:@"Audio"];
    NSArray<NSURL*>* files=[[NSFileManager defaultManager] contentsOfDirectoryAtURL:directory includingPropertiesForKeys:nil options:0 error:nil];
    BOOL ok=files!=nil;
    for(NSURL* url in files){
        NSString* ext=url.pathExtension.lowercaseString;if(![ext isEqualToString:@"wav"])continue;
        std::string name=url.lastPathComponent.UTF8String;if(_sounds.count(name))continue;
        std::vector<std::uint8_t> pcm;ALsizei rate=0;ALenum format=0;
        if(!decode(url,pcm,rate,format)){_lastError=[NSString stringWithFormat:@"Cannot decode %@",url.lastPathComponent];ok=NO;continue;}
        Sound sound;alGenBuffers(1,&sound.buffer);alBufferData(sound.buffer,format,pcm.data(),ALsizei(pcm.size()),rate);
        alGenSources(1,&sound.source);alSourcei(sound.source,AL_BUFFER,sound.buffer);alSourcei(sound.source,AL_SOURCE_RELATIVE,AL_FALSE);
        if(alGetError()!=AL_NO_ERROR){if(sound.source)alDeleteSources(1,&sound.source);if(sound.buffer)alDeleteBuffers(1,&sound.buffer);_lastError=@"OpenAL buffer creation failed";ok=NO;continue;}
        _sounds.emplace(std::move(name),sound);
    }
    if(!ok)NSLog(@"MachDefense audio: %@",_lastError);return ok;
}
- (void)play:(NSString*)name gain:(float)gain pan:(float)pan {
    if(!_active||!name.length||!std::isfinite(gain)||!std::isfinite(pan))return;
    Scope scope(_context);if(!scope.valid)return;
    auto i=_sounds.find(name.UTF8String);if(i==_sounds.end())return;
    const auto source=i->second.source;alSourceStop(source);alSourcef(source,AL_GAIN,std::clamp(gain,0.f,1.f));
    alSource3f(source,AL_POSITION,std::clamp(pan,-1.f,1.f),0,0);alSourcePlay(source);
    if(ALenum error=alGetError();error!=AL_NO_ERROR){_lastError=[NSString stringWithFormat:@"OpenAL play error 0x%x",error];return;}
    ++_playCount;
}
- (void)stopEffects {
    Scope scope(_context);if(!scope.valid)return;
    for(auto& [name,sound]:_sounds)alSourceStop(sound.source);
}
- (void)setActive:(BOOL)active {
    _active=active;
    if(active){
        if(!_context||!(alcGetCurrentContext()==_context||alcMakeContextCurrent(_context)))return;
        alcProcessContext(_context);
        if(_musicName.length&&!_music.playing())_music.start();
    }else{
        {
            Scope scope(_context);if(!scope.valid)return;
            for(auto& [name,sound]:_sounds)alSourceStop(sound.source);
            _music.pause();alcSuspendContext(_context);
        }
        if(alcGetCurrentContext()==_context)alcMakeContextCurrent(nullptr);
    }
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
- (NSUInteger)currentContextIdentity{return reinterpret_cast<NSUInteger>(alcGetCurrentContext());}
- (NSUInteger)loadedSounds{return _sounds.size();}
@end
