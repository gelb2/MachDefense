#include "Audio/OpenALEffects.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <algorithm>
#include <cmath>
#include <map>
namespace md::audio {
namespace {
struct Scope {
    ALCcontext* previous=alcGetCurrentContext();ALCcontext* current=nullptr;bool valid=false;
    explicit Scope(ALCcontext* c):current(c){valid=c&&(previous==c||alcMakeContextCurrent(c));}
    ~Scope(){if(valid&&previous!=current)alcMakeContextCurrent(previous);}
};
struct Sound {ALuint buffer=0,source=0;};
}
struct OpenALEffects::Impl {
    ALCdevice* device=nullptr;ALCcontext* context=nullptr;
    bool active=true;std::map<std::string,Sound,std::less<>> sounds;
    std::uint64_t plays=0;std::string error;
    Impl(){
        device=alcOpenDevice(nullptr);if(device)context=alcCreateContext(device,nullptr);
        if(!context||!alcMakeContextCurrent(context)){error="Cannot initialize OpenAL Soft";return;}
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);alListener3f(AL_POSITION,0,0,0);alListenerf(AL_GAIN,1);
    }
    ~Impl(){
        ALCcontext* previous=alcGetCurrentContext();
        if(context&&alcMakeContextCurrent(context))for(auto& [name,s]:sounds){alSourceStop(s.source);alDeleteSources(1,&s.source);alDeleteBuffers(1,&s.buffer);}
        if(context){alcMakeContextCurrent(previous==context?nullptr:previous);alcDestroyContext(context);}
        if(device)alcCloseDevice(device);
    }
};
OpenALEffects::OpenALEffects():impl_(std::make_unique<Impl>()){}
OpenALEffects::~OpenALEffects()=default;
bool OpenALEffects::valid()const{return impl_->context!=nullptr;}
bool OpenALEffects::contains(std::string_view name)const{return impl_->sounds.contains(name);}
bool OpenALEffects::prepare(std::string name,const PCM& pcm){
    if(contains(name))return true;
    if(pcm.bytes.empty()||pcm.bytes.size()>64*1024*1024||pcm.sampleRate<1||pcm.sampleRate>192000||
       pcm.channels<1||pcm.channels>2||(pcm.bitsPerSample!=8&&pcm.bitsPerSample!=16))return false;
    Scope scope(impl_->context);if(!scope.valid)return false;
    const ALenum format=pcm.bitsPerSample==8?(pcm.channels==1?AL_FORMAT_MONO8:AL_FORMAT_STEREO8):
                                             (pcm.channels==1?AL_FORMAT_MONO16:AL_FORMAT_STEREO16);
    Sound sound;alGenBuffers(1,&sound.buffer);alBufferData(sound.buffer,format,pcm.bytes.data(),ALsizei(pcm.bytes.size()),ALsizei(pcm.sampleRate));
    alGenSources(1,&sound.source);alSourcei(sound.source,AL_BUFFER,sound.buffer);alSourcei(sound.source,AL_SOURCE_RELATIVE,AL_FALSE);
    if(alGetError()!=AL_NO_ERROR){
        if(sound.source)alDeleteSources(1,&sound.source);if(sound.buffer)alDeleteBuffers(1,&sound.buffer);
        impl_->error="OpenAL buffer creation failed";return false;
    }
    impl_->sounds.emplace(std::move(name),sound);return true;
}
void OpenALEffects::play(std::string_view name,float gain,float x,float y){
    if(!impl_->active||name.empty()||!std::isfinite(gain))return;
    Scope scope(impl_->context);if(!scope.valid)return;
    auto i=impl_->sounds.find(name);if(i==impl_->sounds.end())return;
    auto source=i->second.source;alSourceStop(source);alSourcef(source,AL_GAIN,std::clamp(gain,0.f,1.f));
    // Original set-position reports/clears invalid coordinates before play.
    // Retain the source's prior position when the original sqrt yields NaN.
    if(std::isfinite(x)&&std::isfinite(y))alSource3f(source,AL_POSITION,x,y,0);
    alSourcePlay(source);
    if(auto error=alGetError();error!=AL_NO_ERROR){impl_->error="OpenAL play error "+std::to_string(error);return;}
    ++impl_->plays;
}
void OpenALEffects::stop(){Scope scope(impl_->context);if(scope.valid)for(auto& [name,sound]:impl_->sounds)alSourceStop(sound.source);}
void OpenALEffects::setActive(bool active){
    impl_->active=active;
    if(active){if(impl_->context&&(alcGetCurrentContext()==impl_->context||alcMakeContextCurrent(impl_->context)))alcProcessContext(impl_->context);}
    else{
        {Scope scope(impl_->context);if(!scope.valid)return;for(auto& [name,sound]:impl_->sounds)alSourceStop(sound.source);alcSuspendContext(impl_->context);}
        if(alcGetCurrentContext()==impl_->context)alcMakeContextCurrent(nullptr);
    }
}
std::size_t OpenALEffects::loaded()const{return impl_->sounds.size();}
std::uint64_t OpenALEffects::playCount()const{return impl_->plays;}
std::uintptr_t OpenALEffects::currentContextIdentity()const{return reinterpret_cast<std::uintptr_t>(alcGetCurrentContext());}
const std::string& OpenALEffects::error()const{return impl_->error;}
}
