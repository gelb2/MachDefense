#import "MDAudio.h"
#import <AudioToolbox/AudioToolbox.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace {
struct Sound {ALuint buffer=0,source=0;};
struct Scope {
    ALCcontext* previous=alcGetCurrentContext();bool valid=false;
    explicit Scope(ALCcontext* c){valid=c&&alcMakeContextCurrent(c);}
    ~Scope(){if(valid)alcMakeContextCurrent(previous);}
};
bool decode(NSURL* url,std::vector<std::int16_t>& samples,ALsizei& rate,ALenum& format){
    ExtAudioFileRef file=nullptr;if(ExtAudioFileOpenURL((__bridge CFURLRef)url,&file))return false;
    struct Close {ExtAudioFileRef f;~Close(){ExtAudioFileDispose(f);}} close{file};
    AudioStreamBasicDescription source{};UInt32 size=sizeof(source);
    if(ExtAudioFileGetProperty(file,kExtAudioFileProperty_FileDataFormat,&size,&source))return false;
    if(source.mChannelsPerFrame<1||source.mChannelsPerFrame>2||!std::isfinite(source.mSampleRate)||source.mSampleRate<1||source.mSampleRate>192000)return false;
    AudioStreamBasicDescription pcm{};pcm.mSampleRate=source.mSampleRate;pcm.mFormatID=kAudioFormatLinearPCM;
    pcm.mFormatFlags=kAudioFormatFlagIsSignedInteger|kAudioFormatFlagIsPacked;pcm.mChannelsPerFrame=source.mChannelsPerFrame;
    pcm.mBitsPerChannel=16;pcm.mFramesPerPacket=1;pcm.mBytesPerFrame=2*pcm.mChannelsPerFrame;pcm.mBytesPerPacket=pcm.mBytesPerFrame;
    if(ExtAudioFileSetProperty(file,kExtAudioFileProperty_ClientDataFormat,sizeof(pcm),&pcm))return false;
    std::vector<std::int16_t> block(8192*pcm.mChannelsPerFrame);
    while(true){UInt32 frames=8192;AudioBufferList buffers{};buffers.mNumberBuffers=1;buffers.mBuffers[0]={pcm.mChannelsPerFrame,UInt32(block.size()*2),block.data()};
        if(ExtAudioFileRead(file,&frames,&buffers))return false;if(!frames)break;
        if(samples.size()+frames*pcm.mChannelsPerFrame>64*1024*1024/2)return false;
        samples.insert(samples.end(),block.begin(),block.begin()+frames*pcm.mChannelsPerFrame);
    }
    rate=ALsizei(source.mSampleRate);format=source.mChannelsPerFrame==1?AL_FORMAT_MONO16:AL_FORMAT_STEREO16;return !samples.empty();
}
}
@implementation MDAudio {
    NSURL* _root;
    ALCdevice* _device;
    ALCcontext* _context;
    BOOL _active;
    std::map<std::string,Sound> _sounds;
    ALuint _musicSource;
    NSString* _musicName;
}
- (instancetype)initWithResourceRoot:(NSURL*)root {
    if(!(self=[super init]))return nil;_root=root;_active=YES;
    _device=alcOpenDevice(nullptr);if(_device)_context=alcCreateContext(_device,nullptr);
    Scope scope(_context);if(!scope.valid){_lastError=@"Cannot initialize OpenAL";return self;}
    alDistanceModel(AL_NONE);alListener3f(AL_POSITION,0,0,0);alListenerf(AL_GAIN,1);
    alGenSources(1,&_musicSource);alSourcei(_musicSource,AL_SOURCE_RELATIVE,AL_TRUE);
    alSourcei(_musicSource,AL_LOOPING,AL_TRUE);alSourcef(_musicSource,AL_ROLLOFF_FACTOR,0);return self;
}
- (void)dealloc {
    ALCcontext* previous=alcGetCurrentContext();if(_context&&alcMakeContextCurrent(_context)){
        if(_musicSource){alSourceStop(_musicSource);alDeleteSources(1,&_musicSource);}
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
        NSString* ext=url.pathExtension.lowercaseString;if(![ext isEqualToString:@"wav"]&&![ext isEqualToString:@"mp3"])continue;
        std::string name=url.lastPathComponent.UTF8String;if(_sounds.count(name))continue;
        std::vector<std::int16_t> pcm;ALsizei rate=0;ALenum format=0;
        if(!decode(url,pcm,rate,format)){_lastError=[NSString stringWithFormat:@"Cannot decode %@",url.lastPathComponent];ok=NO;continue;}
        Sound sound;alGenBuffers(1,&sound.buffer);alBufferData(sound.buffer,format,pcm.data(),ALsizei(pcm.size()*2),rate);
        alGenSources(1,&sound.source);alSourcei(sound.source,AL_BUFFER,sound.buffer);alSourcei(sound.source,AL_SOURCE_RELATIVE,AL_TRUE);alSourcef(sound.source,AL_ROLLOFF_FACTOR,0);
        if(alGetError()!=AL_NO_ERROR){if(sound.source)alDeleteSources(1,&sound.source);if(sound.buffer)alDeleteBuffers(1,&sound.buffer);_lastError=@"OpenAL buffer creation failed";ok=NO;continue;}
        _sounds.emplace(std::move(name),sound);
    }
    if(!ok)NSLog(@"MachDefense audio: %@",_lastError);return ok;
}
- (void)play:(NSString*)name gain:(float)gain pan:(float)pan {
    if(!_active||!name.length||!std::isfinite(gain)||!std::isfinite(pan))return;
    Scope scope(_context);if(!scope.valid)return;
    auto i=_sounds.find(name.UTF8String);if(i==_sounds.end())return;
    const auto source=i->second.source;alSourceStop(source);alSourceRewind(source);alSourcef(source,AL_GAIN,std::clamp(gain,0.f,1.f));
    alSource3f(source,AL_POSITION,std::clamp(pan,-1.f,1.f),0,-1);alSourcePlay(source);
    if(ALenum error=alGetError();error!=AL_NO_ERROR){_lastError=[NSString stringWithFormat:@"OpenAL play error 0x%x",error];return;}
    ++_playCount;
}
- (void)setActive:(BOOL)active {
    _active=active;Scope scope(_context);if(!scope.valid)return;
    if(!active){for(auto& [name,sound]:_sounds)alSourceStop(sound.source);if(_musicSource)alSourcePause(_musicSource);alcSuspendContext(_context);}
    else {alcProcessContext(_context);if(_musicName.length){ALint state=0;alGetSourcei(_musicSource,AL_SOURCE_STATE,&state);if(state!=AL_PLAYING)alSourcePlay(_musicSource);}}
}
- (void)playMusic:(NSString*)name gain:(float)gain {
    if(!name.length||!std::isfinite(gain))return;
    Scope scope(_context);if(!scope.valid||!_musicSource)return;
    const auto found=_sounds.find(name.UTF8String);if(found==_sounds.end())return;
    if(![_musicName isEqualToString:name]){
        alSourceStop(_musicSource);alSourcei(_musicSource,AL_BUFFER,found->second.buffer);_musicName=[name copy];
    }
    alSourcef(_musicSource,AL_GAIN,std::clamp(gain,0.f,1.f));
    ALint state=0;alGetSourcei(_musicSource,AL_SOURCE_STATE,&state);
    if(_active&&state!=AL_PLAYING)alSourcePlay(_musicSource);
    if(ALenum error=alGetError();error!=AL_NO_ERROR)_lastError=[NSString stringWithFormat:@"OpenAL music error 0x%x",error];
}
- (void)stopMusic {
    Scope scope(_context);if(scope.valid&&_musicSource){alSourceStop(_musicSource);alSourcei(_musicSource,AL_BUFFER,0);}
    _musicName=nil;
}
- (NSString*)musicName{return _musicName;}
- (BOOL)musicPlaying {Scope scope(_context);if(!scope.valid||!_musicSource)return NO;ALint value=0;alGetSourcei(_musicSource,AL_SOURCE_STATE,&value);return value==AL_PLAYING;}
- (BOOL)musicLooping {Scope scope(_context);if(!scope.valid||!_musicSource)return NO;ALint value=0;alGetSourcei(_musicSource,AL_LOOPING,&value);return value==AL_TRUE;}
- (NSUInteger)loadedSounds{return _sounds.size();}
@end
