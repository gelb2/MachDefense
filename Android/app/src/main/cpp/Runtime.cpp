#include "Runtime.hpp"
#include "../../../../../Modules/SaveTransfer/FileStore.hpp"
#include "Core/Simulation/World.hpp"
#include "Core/Simulation/AlliedAircraft.hpp"
#include <android/log.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <stdexcept>
#ifndef MD_VALIDATION
#define MD_VALIDATION 0
#endif
namespace md::android {
namespace {
double now(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
std::string jsonString(std::string_view s){std::string out="\"";for(unsigned char c:s){if(c=='"'||c=='\\'){out+='\\';out+=c;}else if(c>=32)out+=c;else out+=' ';}return out+'"';}
}
Runtime::Runtime(AAssetManager* manager,std::string directory,Callbacks callbacks):assets_(manager),renderer_(assets_),directory_(std::move(directory)),callbacks_(std::move(callbacks)){
    if(!effects_.valid())throw std::runtime_error(effects_.error());
    for(const auto& file:assets_.list("Audio"))if(file.ends_with(".wav")){
        auto bytes=assets_.read("Audio/"+file);if(!effects_.prepare(file,decodeWav(bytes)))throw std::runtime_error(effects_.error());
    }
    effects_.setActive(false);game_=createGame(assets_,*this,MD_VALIDATION);game_->setActive(false);samples_.reserve(18000);
}
Runtime::~Runtime(){cancelTouches();game_.reset();stopMusic();effects_.setActive(false);releaseSurface();if(window_)ANativeWindow_release(window_);}
void Runtime::releaseSurface(){
    if(display_!=EGL_NO_DISPLAY){
        if(context_!=EGL_NO_CONTEXT&&surface_!=EGL_NO_SURFACE&&eglMakeCurrent(display_,surface_,surface_,context_))renderer_.release();else renderer_.lostContext();
        eglMakeCurrent(display_,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        if(surface_!=EGL_NO_SURFACE)eglDestroySurface(display_,surface_);
        if(context_!=EGL_NO_CONTEXT)eglDestroyContext(display_,context_);
        eglTerminate(display_);
    }
    display_=EGL_NO_DISPLAY;surface_=EGL_NO_SURFACE;context_=EGL_NO_CONTEXT;previousTime_=0;
}
void Runtime::surface(ANativeWindow* window){
    cancelTouches();if(window)ANativeWindow_acquire(window);releaseSurface();if(window_)ANativeWindow_release(window_);window_=window;if(!window)return;
    display_=eglGetDisplay(EGL_DEFAULT_DISPLAY);if(display_==EGL_NO_DISPLAY||!eglInitialize(display_,nullptr,nullptr))throw std::runtime_error("EGL initialization failed");
    const EGLint attributes[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,0,EGL_STENCIL_SIZE,0,EGL_NONE};
    EGLConfig config;EGLint count=0;if(!eglChooseConfig(display_,attributes,&config,1,&count)||count!=1)throw std::runtime_error("EGL config unavailable");
    EGLint format;eglGetConfigAttrib(display_,config,EGL_NATIVE_VISUAL_ID,&format);ANativeWindow_setBuffersGeometry(window,0,0,format);
    const EGLint contextAttributes[]={EGL_CONTEXT_CLIENT_VERSION,1,EGL_NONE};context_=eglCreateContext(display_,config,EGL_NO_CONTEXT,contextAttributes);
    surface_=eglCreateWindowSurface(display_,config,window,nullptr);
    if(context_==EGL_NO_CONTEXT||surface_==EGL_NO_SURFACE||!eglMakeCurrent(display_,surface_,surface_,context_))throw std::runtime_error("EGL surface unavailable");
    eglSwapInterval(display_,1);eglQuerySurface(display_,surface_,EGL_WIDTH,&width_);eglQuerySurface(display_,surface_,EGL_HEIGHT,&height_);++contexts_;resize();
}
void Runtime::insets(int top,int bottom,float density){top_=std::max(0,top);bottom_=std::max(0,bottom);density_=std::max(1.f,density);resize();}
void Runtime::resize(){if(game_&&width_>0&&height_>0){game_->resize(width_/density_,height_/density_,top_/density_,bottom_/density_);if(transferVisible_)layoutTransferField(transferBounds_);}}
void Runtime::cancelTouches(){auto old=std::move(touches_);touches_.clear();if(game_)for(auto [id,t]:old){t.phase=TouchPhase::cancelled;game_->touch(t);}}
void Runtime::updateAudio(){effects_.setActive(active_&&audioFocus_);callbacks_.musicActive(active_&&audioFocus_);}
void Runtime::active(bool on){active_=on;previousTime_=0;if(!on)cancelTouches();if(game_)game_->setActive(on);updateAudio();}
void Runtime::focus(bool on){audioFocus_=on;if(!on&&game_){cancelTouches();game_->setActive(false);game_->setActive(active_);}updateAudio();}
void Runtime::frame(double time){
    if(!active_||surface_==EGL_NO_SURFACE||!game_||!error.empty())return;
    const auto started=now();const auto uploads=renderer_.textureUploads;const auto sounds=effects_.playCount();const auto writes=writes_;
    if(!nameCompletion_)game_->tick();const auto ticked=now();
    renderer_.draw(game_->frame(),width_,height_);const auto drawn=now();
    if(!eglSwapBuffers(display_,surface_)){
        const auto e=eglGetError();if(e==EGL_CONTEXT_LOST||e==EGL_BAD_SURFACE){game_->setActive(false);game_->setActive(active_);surface(window_);return;}
        throw std::runtime_error("EGL swap failed");
    }
    if(auditing_&&samples_.size()<18000)samples_.push_back({previousTime_?(time-previousTime_)*1000:0,(ticked-started)*1000,(drawn-ticked)*1000,(now()-drawn)*1000,renderer_._metrics.sprites,renderer_.textureUploads-uploads,unsigned(effects_.playCount()-sounds),writes_-writes});
    previousTime_=time;++frames_;
}
void Runtime::touch(unsigned id,int phase,float x,float y){
    if(!active_||!game_||nameCompletion_)return;
    auto point=renderer_.logicalPoint(x,y);auto found=touches_.find(id);
    if(phase==0){if(!point)return;Touch t{id,TouchPhase::began,*point};touches_[id]=t;game_->touch(t);}
    else if(found!=touches_.end()){auto t=found->second;t.phase=point?TouchPhase(phase):TouchPhase::cancelled;if(point)t.position=*point;
        if(t.phase==TouchPhase::ended||t.phase==TouchPhase::cancelled)touches_.erase(found);else found->second=t;game_->touch(t);}
}
std::optional<Bytes> Runtime::readSave(){
    if(!transfer::FileStore(directory_+"/MachDefense.sav").recover())throw std::runtime_error("Cannot finish save recovery");
    restoreBlocked_=false;
    ++reads_;auto path=directory_+"/MachDefense.sav";FILE* file=fopen(path.c_str(),"rb");if(!file){if(errno==ENOENT)return {};throw std::runtime_error("Cannot read save");}
    struct Close{FILE*p;~Close(){fclose(p);}} close{file};
    if(fseek(file,0,SEEK_END))throw std::runtime_error("Cannot seek save");auto length=ftell(file);if(length<0||length>1024*1024)throw std::runtime_error("Invalid save size");rewind(file);
    Bytes bytes(static_cast<std::size_t>(length));if(fread(bytes.data(),1,bytes.size(),file)!=bytes.size())throw std::runtime_error("Cannot read complete save");return bytes;
}
bool Runtime::writeSave(std::span<const std::uint8_t> bytes){
    if(restoreBlocked_)return false;
    if(bytes.empty())return false;auto dest=directory_+"/MachDefense.sav",temp=dest+".tmp";
    int fd=open(temp.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);if(fd<0)return false;
    bool ok=true;std::size_t pos=0;while(pos<bytes.size()){auto n=write(fd,bytes.data()+pos,bytes.size()-pos);if(n<0&&errno==EINTR)continue;if(n<=0){ok=false;break;}pos+=n;}
    if(ok&&fsync(fd))ok=false;if(close(fd))ok=false;if(ok&&rename(temp.c_str(),dest.c_str()))ok=false;
    if(ok){int dir=open(directory_.c_str(),O_RDONLY|O_DIRECTORY|O_CLOEXEC);if(dir<0)ok=false;else{if(fsync(dir))ok=false;close(dir);}}
    if(!ok)unlink(temp.c_str());else ++writes_;return ok;
}
void Runtime::playSound(std::string_view name,float gain,float pan){if(std::isfinite(pan))effects_.play(name,gain,std::clamp(pan,-1.f,1.f),0);}
void Runtime::playSoundAt(std::string_view name,Vec2 p,float gain){effects_.play(name,gain,p.x,p.y);}
void Runtime::playMusic(std::string_view name,float gain){callbacks_.music(std::string(name),gain);}
void Runtime::stopMusic(){callbacks_.music("",0);}
void Runtime::requestName(std::string name,std::size_t max,std::function<void(std::optional<std::string>)> cb){cancelTouches();nameCompletion_=std::move(cb);callbacks_.name(std::move(name),unsigned(max),++nameGeneration_);}
Rect Runtime::transferPixelBounds()const{
    const auto& frame=game_->frame();
    const float scale=std::min(width_/frame.logicalWidth,height_/frame.logicalHeight);
    const float w=std::round(frame.logicalWidth*scale),h=std::round(frame.logicalHeight*scale);
    const float sx=w/frame.logicalWidth,sy=h/frame.logicalHeight;
    return {(width_-w)/2+transferBounds_.x*sx,(height_-h)/2+transferBounds_.y*sy,transferBounds_.width*sx,transferBounds_.height*sy};
}
void Runtime::openTransferField(std::string value,bool readOnly,Rect bounds){
    cancelTouches();transferText_=std::move(value);transferReadOnly_=readOnly;transferBounds_=bounds;transferVisible_=true;
    if(callbacks_.transferOpen)callbacks_.transferOpen(transferText_,readOnly,transferPixelBounds(),++transferGeneration_);
}
void Runtime::layoutTransferField(Rect bounds){
    transferBounds_=bounds;
    if(transferVisible_&&callbacks_.transferLayout)callbacks_.transferLayout(transferPixelBounds(),transferGeneration_);
}
void Runtime::closeTransferField(){
    if(!transferVisible_)return;
    transferVisible_=false;transferText_.clear();
    if(callbacks_.transferClose)callbacks_.transferClose(transferGeneration_);
    ++transferGeneration_;
}
void Runtime::transferTextChanged(std::uint64_t generation,std::string value){
    if(!transferVisible_||transferReadOnly_||generation!=transferGeneration_)return;
    // Reject the newly entered value; never leave an older valid code cached.
    transferText_=value.size()<=120000?std::move(value):std::string{};
}
void Runtime::copyTransferCode(std::string value){if(callbacks_.transferCopy)callbacks_.transferCopy(std::move(value));}
bool Runtime::restoreSave(std::span<const std::uint8_t> bytes){
    if(restoreBlocked_)throw std::runtime_error("Close and reopen the app to finish save recovery.");
    try{return transfer::FileStore(directory_+"/MachDefense.sav").restore(bytes);}
    catch(...){restoreBlocked_=true;throw;}
}
void Runtime::nameResult(std::uint64_t generation,std::optional<std::string> value){if(!nameCompletion_||generation!=nameGeneration_)return;auto cb=std::move(nameCompletion_);nameCompletion_={};cb(std::move(value));}
void Runtime::command(const std::string& name,int a,int b){
#if MD_VALIDATION
    auto& session=static_cast<GameSession&>(*game_);
    if(name=="context-cycle"){surface(window_);}
    else if(name=="validation")session.applyValidationCommand(unsigned(a));
    else if(name=="tap"){Touch t{999,TouchPhase::began,{float(a),float(b)}};session.touch(t);t.phase=TouchPhase::ended;session.touch(t);}
    else if(name=="ticks"){for(int i=0;i<std::clamp(a,0,10000);++i)session.tick();}
    else if(name=="audio")playSound("HMG09.wav",.2f,0);
    else if(name=="focus")focus(a!=0);
    else if(name=="audit-start"){samples_.clear();previousTime_=0;auditing_=true;}
    else if(name=="audit-stop"){
        auditing_=false;std::ofstream out(directory_+"/frame-audit.csv");out<<"interval_ms,tick_ms,draw_ms,swap_ms,sprites,uploads,sounds,writes\n";
        for(const auto& s:samples_)out<<s.interval<<','<<s.tick<<','<<s.draw<<','<<s.swap<<','<<s.sprites<<','<<s.uploads<<','<<s.sounds<<','<<s.writes<<'\n';
    }else if(name=="snapshot"){std::ofstream out(directory_+"/validation.json");out<<status(true);}
#else
    (void)name;(void)a;(void)b;
#endif
}
std::string Runtime::status(bool sprites){
    std::ostringstream out;out<<"{\"error\":"<<jsonString(error)<<",\"frames\":"<<frames_<<",\"contexts\":"<<contexts_<<",\"uploads\":"<<renderer_.textureUploads<<",\"audioBuffers\":"<<effects_.loaded()<<",\"sounds\":"<<effects_.playCount()<<",\"audioError\":"<<jsonString(effects_.error())<<",\"reads\":"<<reads_<<",\"writes\":"<<writes_<<",\"active\":"<<(active_?"true":"false")<<",\"focus\":"<<(audioFocus_?"true":"false");
    out<<",\"transferVisible\":"<<(transferVisible_?"true":"false")<<",\"transferReadOnly\":"<<(transferReadOnly_?"true":"false")<<",\"transferLength\":"<<transferText_.size()<<",\"transferGeneration\":"<<transferGeneration_;
    if(game_){const auto& f=game_->frame();out<<",\"width\":"<<f.logicalWidth<<",\"height\":"<<f.logicalHeight<<",\"pixelWidth\":"<<width_<<",\"pixelHeight\":"<<height_;
#if MD_VALIDATION
        out<<",\"probeSeconds\":"<<effects_.playbackTime("HMG09.wav");
        auto& session=static_cast<GameSession&>(*game_);if(auto* w=session.validationWorld())out<<",\"world\":{\"stage\":"<<w->stage()<<",\"time\":"<<w->time()<<",\"paused\":"<<(w->paused()?"true":"false")<<",\"cells\":"<<w->cells()<<",\"actors\":"<<w->actors().size()<<",\"projectiles\":"<<w->projectiles().size()<<",\"produced\":"<<w->statistics().produced<<",\"state\":"<<int(w->state())<<'}';
        if(auto save=session.validationSave()){out<<",\"slots\":[";for(unsigned i=0;i<5;++i){if(i)out<<',';out<<"{\"active\":"<<(save->active(i)?"true":"false")<<",\"stage\":"<<save->stage(i)<<",\"credits\":"<<save->credits(i)<<'}';}out<<']';}
        // Detailed reads run only for the explicit snapshot request (status(true)).
        if(sprites){
            const auto helicopterImage=[](std::string_view image){return image.starts_with("ALight")||image.starts_with("AHeavy")||image.starts_with("ADuct")||image.starts_with("ATransport");};
            out<<",\"sprites\":[";bool comma=false;
            for(const auto& s:f.sprites){if(comma)out<<',';comma=true;
                out<<"{\"image\":"<<jsonString(s.image)<<",\"x\":"<<s.destination.x<<",\"y\":"<<s.destination.y<<",\"w\":"<<s.destination.width<<",\"h\":"<<s.destination.height;
                if(helicopterImage(s.image)){
                    out<<",\"rotation\":"<<s.rotationDegrees<<",\"color\":["<<s.color.r<<','<<s.color.g<<','<<s.color.b<<','<<s.color.a<<"],\"sourcePx\":";
                    if(s.sourcePx){const auto& r=*s.sourcePx;out<<'['<<r.x<<','<<r.y<<','<<r.width<<','<<r.height<<']';}else out<<"null";
                }
                out<<'}';
            }out<<']';
            out<<",\"aircraftMeshes\":[";comma=false;
            for(const auto& m:f.meshes)if(helicopterImage(m.image)){
                if(comma)out<<',';comma=true;out<<"{\"image\":"<<jsonString(m.image)<<",\"beforeSpriteIndex\":"<<m.beforeSpriteIndex<<",\"vertices\":[";bool vertexComma=false;
                for(const auto& v:m.vertices){if(vertexComma)out<<',';vertexComma=true;out<<"{\"position\":["<<v.position.x<<','<<v.position.y<<"],\"uv\":["<<v.uv.x<<','<<v.uv.y<<"],\"color\":["<<v.color.r<<','<<v.color.g<<','<<v.color.b<<','<<v.color.a<<"]}";}
                out<<"]}";
            }out<<']';
            out<<",\"aircraftBuilds\":[";comma=false;
            if(auto save=session.validationSave())for(unsigned slot=0;slot<SaveData::slotCount;++slot)if(save->active(slot))for(const auto& b:save->builds(slot)){
                const bool aircraft=std::any_of(alliedAircraftCatalog.begin(),alliedAircraftCatalog.end(),[&](const auto& a){return a.name==b.name;});if(!aircraft)continue;
                if(comma)out<<',';comma=true;out<<"{\"slot\":"<<slot<<",\"name\":"<<jsonString(b.name)<<",\"savedLevel\":"<<b.level<<",\"displayLevel\":"<<b.level+1<<",\"onSlot\":"<<(b.onSlot?"true":"false")<<'}';
            }out<<']';
            out<<",\"actorsDetail\":[";comma=false;
            if(auto* w=session.validationWorld()){
                const auto bytes=assets_.read("Tables/WeaponInfo.tsv");
                const auto catalog=WeaponCatalog::load(DataTable::parse({reinterpret_cast<const char*>(bytes.data()),bytes.size()}),true);
                const auto models=BattleModels::load(assets_);
                for(const auto& ptr:w->actors())if(ptr->flightTactics){const auto& a=*ptr;
                    if(comma)out<<',';comma=true;
                    out<<"{\"name\":"<<jsonString(a.name)<<",\"id\":"<<a.id<<",\"hp\":"<<a.hp<<",\"maxHP\":"<<a.maxHP<<",\"radius\":"<<a.radius<<",\"range\":"<<a.range<<",\"moveSpeed\":"<<a.aircraft.moveSpeed<<",\"speed\":"<<a.aircraft.speed<<",\"tacticSpeed\":"<<a.flightTactics->speed<<",\"bank\":"<<a.flightTactics->bank<<",\"removed\":"<<(a.removed?"true":"false")<<",\"position\":["<<a.position.x<<','<<a.position.y<<"],\"target\":"<<a.target;
                    out<<",\"parts\":[";bool partComma=false;
                    const auto appendParts=[&](auto&& self,const auto& nodes,const std::string& parentPath)->void{
                        for(std::size_t i=0;i<nodes.size();++i){const auto& p=nodes[i];const auto path=parentPath+"/"+std::to_string(i);if(partComma)out<<',';partComma=true;
                            out<<"{\"path\":"<<jsonString(path)<<",\"name\":"<<jsonString(p.name)<<",\"parentSocket\":"<<unsigned(p.parentSocket)<<",\"zOrder\":"<<unsigned(p.zOrder);
                            if(const auto* definition=models.part(p.name))out<<",\"packagedImage\":"<<jsonString(definition->image)<<",\"type\":"<<unsigned(definition->type)<<",\"tileCount\":"<<unsigned(definition->tileCount)<<",\"tileNo\":"<<definition->tileNo<<",\"grid\":["<<definition->splitX<<','<<definition->splitY<<']';
                            out<<'}';self(self,p.children,path);
                        }
                    };appendParts(appendParts,a.assembly,"");out<<']';
                    out<<",\"weapons\":[";bool weaponComma=false;
                    for(const auto& gun:a.weapons){if(weaponComma)out<<',';weaponComma=true;out<<"{\"name\":"<<jsonString(gun.name)<<",\"partPath\":[";
                        for(std::size_t i=0;i<gun.partPath.size();++i){if(i)out<<',';out<<gun.partPath[i];}out<<"],\"muzzleIndex\":"<<gun.muzzle<<",\"angle\":"<<gun.controller.angle();
                        out<<",\"definitionSource\":\"packaged WeaponInfo via shared phone parser\",\"definition\":";
                        if(const auto* d=catalog.find(gun.name))out<<"{\"damage\":"<<d->damage<<",\"damageRange\":"<<d->damageRange<<",\"shotRange\":"<<d->shotRange<<",\"shotCount\":"<<unsigned(d->shotCount)<<",\"shotDelay\":"<<d->shotDelay<<",\"reloadDelay\":"<<d->reloadDelay<<",\"speed\":"<<d->speed<<",\"attackPoint\":"<<d->attackPoint()<<",\"fireSound\":"<<jsonString(d->fireSound)<<'}';else out<<"null";
                        out<<'}';
                    }out<<"]}";
                }
            }out<<']';
        }
#else
        (void)sprites;
#endif
    }
    out<<'}';return out.str();
}
}
