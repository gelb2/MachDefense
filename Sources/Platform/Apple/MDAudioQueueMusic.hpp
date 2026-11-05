#pragma once
#include <AudioToolbox/AudioToolbox.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace md {
// Original BGM: 0x49bb8 AudioQueueNewOutput, 0x49d38 three half-second
// packet buffers, 0x49eb0 refill/loop. Only music uses this Apple adapter.
class AudioQueueMusic {
    AudioFileID file_=nullptr;
    AudioQueueRef queue_=nullptr;
    AudioStreamBasicDescription format_{};
    std::array<AudioQueueBufferRef,3> buffers_{};
    std::vector<AudioStreamPacketDescription> descriptions_;
    SInt64 packet_=0;
    UInt32 packetsPerBuffer_=0,bufferBytes_=0;
    OSStatus error_=noErr;
    bool playing_=false;
    float gain_=1;
    static void refill(void* context,AudioQueueRef queue,AudioQueueBufferRef buffer){
        static_cast<AudioQueueMusic*>(context)->fill(queue,buffer);
    }
    void fill(AudioQueueRef queue,AudioQueueBufferRef buffer){
        if(error_||queue!=queue_||!file_)return;
        UInt32 packets=packetsPerBuffer_,bytes=bufferBytes_;
        auto* descriptions=descriptions_.empty()?nullptr:descriptions_.data();
        error_=AudioFileReadPacketData(file_,false,&bytes,descriptions,packet_,&packets,buffer->mAudioData);
        if(error_)return;
        if(!packets){
            packet_=0;packets=packetsPerBuffer_;bytes=bufferBytes_;
            error_=AudioFileReadPacketData(file_,false,&bytes,descriptions,packet_,&packets,buffer->mAudioData);
        }
        if(error_||!packets)return;
        packet_+=packets;buffer->mAudioDataByteSize=bytes;
        error_=AudioQueueEnqueueBuffer(queue,buffer,descriptions?packets:0,descriptions);
    }
public:
    AudioQueueMusic()=default;
    AudioQueueMusic(const AudioQueueMusic&)=delete;
    AudioQueueMusic& operator=(const AudioQueueMusic&)=delete;
    ~AudioQueueMusic(){close();}
    void close(){
        playing_=false;
        if(queue_){AudioQueueStop(queue_,true);AudioQueueDispose(queue_,true);queue_=nullptr;}
        if(file_){AudioFileClose(file_);file_=nullptr;}
        buffers_={};descriptions_.clear();packet_=0;
    }
    bool open(CFURLRef url){
        close();error_=AudioFileOpenURL(url,kAudioFileReadPermission,0,&file_);
        if(error_)return false;
        UInt32 size=sizeof(format_);
        error_=AudioFileGetProperty(file_,kAudioFilePropertyDataFormat,&size,&format_);if(error_)return false;
        UInt32 maxPacket=0;size=sizeof(maxPacket);
        error_=AudioFileGetProperty(file_,kAudioFilePropertyPacketSizeUpperBound,&size,&maxPacket);
        if(error_||!maxPacket||maxPacket>1024*1024)return false;
        bufferBytes_=std::max(maxPacket,UInt32(std::clamp(
            format_.mFramesPerPacket?std::ceil(format_.mSampleRate*.5/format_.mFramesPerPacket)*maxPacket:65536.,16384.,65536.)));
        packetsPerBuffer_=std::max(1u,bufferBytes_/maxPacket);
        if(!format_.mBytesPerPacket||!format_.mFramesPerPacket)descriptions_.resize(packetsPerBuffer_);
        error_=AudioQueueNewOutput(&format_,refill,this,CFRunLoopGetCurrent(),kCFRunLoopCommonModes,0,&queue_);
        if(error_)return false;
        for(auto props:{std::pair{ kAudioFilePropertyMagicCookieData,kAudioQueueProperty_MagicCookie},
                        std::pair{ kAudioFilePropertyChannelLayout,kAudioQueueProperty_ChannelLayout}}){
            UInt32 n=0;
            if(AudioFileGetPropertyInfo(file_,props.first,&n,nullptr)==noErr&&n){
                std::vector<std::uint8_t> data(n);
                error_=AudioFileGetProperty(file_,props.first,&n,data.data());if(error_)return false;
                error_=AudioQueueSetProperty(queue_,props.second,data.data(),n);if(error_)return false;
            }
        }
        for(auto& buffer:buffers_){
            error_=AudioQueueAllocateBuffer(queue_,bufferBytes_,&buffer);if(error_)return false;
            fill(queue_,buffer);if(error_)return false;
        }
        return packet_>0;
    }
    bool start(){if(!queue_||error_)return false;error_=AudioQueueStart(queue_,nullptr);playing_=error_==noErr;return playing_;}
    void pause(){if(queue_){error_=AudioQueuePause(queue_);playing_=false;}}
    void setGain(float gain){gain_=gain;if(queue_)error_=AudioQueueSetParameter(queue_,kAudioQueueParam_Volume,gain);}
    bool playing()const {return playing_&&queue_&&error_==noErr;}
    float gain()const{return gain_;}
    float time()const{
        AudioTimeStamp stamp{};Boolean discontinuity=false;
        if(!queue_||AudioQueueGetCurrentTime(queue_,nullptr,&stamp,&discontinuity)!=noErr||format_.mSampleRate<=0)return 0;
        return float(stamp.mSampleTime/format_.mSampleRate);
    }
    OSStatus error()const{return error_;}
};
}
