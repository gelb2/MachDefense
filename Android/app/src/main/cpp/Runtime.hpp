#pragma once
#include "Renderer.hpp"
#include "Audio/OpenALEffects.hpp"
#include "Core/Session/GameSession.hpp"
#include <EGL/egl.h>
#include <android/native_window.h>
namespace md::android {
struct Callbacks {
    std::function<void(std::string,float)> music;
    std::function<void(bool)> musicActive;
    std::function<void(std::string,unsigned,std::uint64_t)> name;
};
class Runtime final:public HostServices {
    Assets assets_;Renderer renderer_;audio::OpenALEffects effects_;
    std::unique_ptr<Game> game_;std::string directory_;Callbacks callbacks_;
    ANativeWindow* window_=nullptr;
    EGLDisplay display_=EGL_NO_DISPLAY;EGLSurface surface_=EGL_NO_SURFACE;EGLContext context_=EGL_NO_CONTEXT;
    bool active_=false,audioFocus_=false;int width_=0,height_=0,top_=0,bottom_=0;float density_=1;
    std::map<unsigned,Touch> touches_;
    std::function<void(std::optional<std::string>)> nameCompletion_;std::uint64_t nameGeneration_=0;
    struct Sample{double interval=0,tick=0,draw=0,swap=0;unsigned sprites=0,uploads=0,sounds=0,writes=0;};
    std::vector<Sample> samples_;double previousTime_=0;bool auditing_=false;
    unsigned writes_=0,reads_=0;std::uint64_t frames_=0,contexts_=0;
    void releaseSurface();void cancelTouches();void resize();void updateAudio();
public:
    std::string error;
    Runtime(AAssetManager*,std::string,Callbacks);
    ~Runtime();
    void surface(ANativeWindow*);
    void insets(int,int,float);
    void active(bool);void focus(bool);void frame(double);void touch(unsigned,int,float,float);
    void nameResult(std::uint64_t,std::optional<std::string>);
    void command(const std::string&,int,int);
    std::string status(bool includeSprites=false);
    std::optional<Bytes> readSave()override;
    bool writeSave(std::span<const std::uint8_t>)override;
    void playSound(std::string_view,float,float)override;
    void playSoundAt(std::string_view,Vec2,float)override;
    void playMusic(std::string_view,float)override;
    void stopMusic()override;
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override;
};
}
