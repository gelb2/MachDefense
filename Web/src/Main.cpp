#include "Assets.hpp"
#include "Audio.hpp"
#include "Renderer.hpp"
#include "Core/Session/GameSession.hpp"
#include "Core/Simulation/World.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#if MD_WEB_CHEAT
#include "../../Tests/Level10HUDSweep.hpp"
#include "../../Tests/ContentSweep.hpp"
#include "../../Tests/TurretArtSweep.hpp"
#include "../../Tests/AircraftComparisonScene.hpp"
#include "../../Tests/AircraftVisualFrame.hpp"
#endif
#include <sstream>
#include <cmath>
using namespace md;
EM_ASYNC_JS(int, commitSave,(const unsigned char* p,int n,int restore),{try{return await window.MDStore.write(HEAPU8.slice(p,p+n),!!restore)?1:0;}catch(e){console.error(e);return 0;}});
EM_JS(int, saveLength,(),{return window.MDStore.current?.length||0;});
EM_JS(void, copySave,(unsigned char* p),{HEAPU8.set(window.MDStore.current,p);});
EM_JS(void, music,(const char* n,float gain),{window.MDAudio.music(UTF8ToString(n),gain);});
EM_ASYNC_JS(char*, inputName,(const char* initial,int maximum),{const result=await window.MDText.input('PROFILE NAME',UTF8ToString(initial),maximum);if(result===null)return 0;return stringToNewUTF8(result);});
EM_JS(void, openTransfer,(const char* text,int readOnly,float x,float y,float w,float h),{window.MDTransfer.open(UTF8ToString(text),!!readOnly,{x,y,w,h});});
EM_JS(void, layoutTransfer,(float x,float y,float w,float h),{window.MDTransfer.bounds={x,y,w,h};window.MDTransfer.layout();});
EM_JS(char*, readTransfer,(),{return stringToNewUTF8(window.MDTransfer.text());});
EM_JS(void, closeTransfer,(),{window.MDTransfer.close();});
EM_JS(void, copyTransfer,(const char* text),{window.MDTransfer.copy(UTF8ToString(text));});
EM_JS(void, transferViewport,(float w,float h),{window.MDTransfer.viewport={w,h};window.MDTransfer.layout();});

namespace {
web::Assets assets;web::Audio effects;web::Renderer renderer(assets);std::unique_ptr<GameSession> game;bool contextLost=false;int pw=320,ph=480;unsigned ticks=0;
struct Host:HostServices{
 unsigned writes=0;
 std::optional<Bytes> readSave()override{int n=saveLength();if(!n)return {};Bytes b(n);copySave(b.data());return b;}
 bool writeSave(std::span<const uint8_t> b)override{bool ok=commitSave(b.data(),b.size(),0);if(ok)++writes;return ok;}
 void playSound(std::string_view n,float g,float pan)override{effects.play(n,g,pan,0);}
 void playSoundAt(std::string_view n,Vec2 pos,float g)override{effects.play(n,g,pos.x,pos.y);}
 void playMusic(std::string_view n,float g)override{music(std::string(n).c_str(),g);}
 void stopMusic()override{music("",0);}
 bool restoreSave(std::span<const uint8_t> b)override{bool ok=commitSave(b.data(),b.size(),1);if(ok)++writes;return ok;}
 bool supportsSaveTransfer()const override{return true;}
 void openTransferField(std::string initial,bool readOnly,Rect r)override{openTransfer(initial.c_str(),readOnly,r.x,r.y,r.width,r.height);}
 void layoutTransferField(Rect r)override{layoutTransfer(r.x,r.y,r.width,r.height);}
 std::string transferFieldText()const override{char* p=readTransfer();std::string result=p?p:"";free(p);return result;}
 void closeTransferField()override{closeTransfer();}
 void copyTransferCode(std::string text)override{copyTransfer(text.c_str());}
 void requestName(std::string initial,std::size_t max,std::function<void(std::optional<std::string>)> cb)override{char* n=inputName(initial.c_str(),max);if(n){std::string s=n;free(n);cb(s);}else cb({});}
} host;
std::string status;
#if MD_WEB_CHEAT
std::unique_ptr<validation::AircraftComparisonScene> aircraftScene;
#endif
}
extern "C" {
EMSCRIPTEN_KEEPALIVE int md_start(){try{EmscriptenWebGLContextAttributes attr;emscripten_webgl_init_context_attributes(&attr);attr.majorVersion=2;attr.alpha=false;attr.depth=false;attr.stencil=false;attr.antialias=false;auto ctx=emscripten_webgl_create_context("#game",&attr);if(ctx<=0)throw std::runtime_error("WebGL 2 is unavailable");emscripten_webgl_make_context_current(ctx);renderer.init();effects.prepare(assets);game=std::make_unique<GameSession>(assets,host,MD_WEB_CHEAT);return 1;}catch(const std::exception& e){status=e.what();return 0;}}
EMSCRIPTEN_KEEPALIVE void md_tick(){if(!game||contextLost)return;game->tick();++ticks;renderer.draw(game->frame(),pw,ph);}
EMSCRIPTEN_KEEPALIVE void md_touch(int id,int phase,float x,float y){if(game&&!contextLost)game->touch({std::uint64_t(id),TouchPhase(phase),{x*game->frame().logicalWidth,y*game->frame().logicalHeight}});}
EMSCRIPTEN_KEEPALIVE int md_scroll(float x,float y,float deltaY){return game&&!contextLost&&game->scrollProduction({x*game->frame().logicalWidth,y*game->frame().logicalHeight},deltaY*game->frame().logicalHeight);}
EMSCRIPTEN_KEEPALIVE const char* md_level10_sweep(){
#if MD_WEB_CHEAT
 try { status=validation::runLevel10HUDSweep(assets,&host,[](const Frame& frame,std::string_view){renderer.draw(frame,pw,ph);}); }
 catch(const std::exception& e){status=std::string("ERROR: ")+e.what();}
#else
 status="Unavailable";
#endif
 return status.c_str();
}
EMSCRIPTEN_KEEPALIVE const char* md_content_sweep(){
#if MD_WEB_CHEAT
 try { validation::Level10SweepHost isolated(&host);unsigned frames=0;
 auto result=validation::runContentSweep(assets,isolated,[&](const Frame& frame){if(frames++%16==0)renderer.draw(frame,pw,ph);});
 std::ostringstream out;out<<"Content: builds="<<result.builds<<" placements="<<result.placements<<" specials="<<result.attacks<<" frames="<<frames<<" writes="<<isolated.writes;status=out.str(); }
 catch(const std::exception& e){status=std::string("ERROR: ")+e.what();}
#else
 status="Unavailable";
#endif
 return status.c_str();
}
EMSCRIPTEN_KEEPALIVE const char* md_art_frame(unsigned family){
#if MD_WEB_CHEAT
 try { auto frame=validation::turretArtFrame(assets,family);renderer.draw(frame,pw,ph);status="Art: 24 poses; level-five weapons; body levels; canvases/sockets/muzzles unchanged"; }
 catch(const std::exception& e){status=std::string("ERROR: ")+e.what();}
#else
 status="Unavailable";
#endif
 return status.c_str();
}
EMSCRIPTEN_KEEPALIVE const char* md_aircraft_scene(int command,int mode){
#if MD_WEB_CHEAT
 try {
  if(command==8){renderer.draw(validation::aircraftVisualFrame(assets,unsigned(mode)),pw,ph);status="visual comparison at uniform 2x";return status.c_str();}
  if(command==20||command==21){aircraftScene=std::make_unique<validation::AircraftComparisonScene>(assets,3);aircraftScene->production(command==20?unsigned(mode):10,command==20?4:unsigned(std::clamp(mode,1,100)),command==20);}
  if(command==0||!aircraftScene)aircraftScene=std::make_unique<validation::AircraftComparisonScene>(assets,unsigned(mode));
  aircraftScene->host.audio=[](std::string_view name,float gain,float pan){effects.play(name,gain,pan);};
  if(command==22)aircraftScene->world->validationCountermeasureBurst();
  if(command==1)aircraftScene->world->tick();
  if(command==2)aircraftScene->world->setPaused(true);
  if(command==3)aircraftScene->world->setPaused(false);
  if(command==4)aircraftScene->move();
  if(command==6)aircraftScene->fight();
  if(command==7)aircraftScene->world->validationAircraftDodgeComparison();
  renderer.draw(aircraftScene->frame(),pw,ph);status=aircraftScene->state();
 } catch(const std::exception& e){status=std::string("ERROR: ")+e.what();}
#else
 status="Unavailable";
#endif
 return status.c_str();
}
EMSCRIPTEN_KEEPALIVE void md_resize(int w,int h,int pixelw,int pixelh,float top,float bottom){pw=pixelw;ph=pixelh;if(game){game->resize(w,h,top,bottom);transferViewport(game->frame().logicalWidth,game->frame().logicalHeight);}}
EMSCRIPTEN_KEEPALIVE void md_active(int active){effects.setActive(active);if(game)game->setActive(active);}
EMSCRIPTEN_KEEPALIVE void md_context(int restored){contextLost=!restored;if(restored)renderer.init();else renderer.lost();if(game)game->setActive(false);}
EMSCRIPTEN_KEEPALIVE const char* md_debug(){std::ostringstream o;o<<"{\"ready\":"<<(game?"true":"false")<<",\"cheat\":"<<(MD_WEB_CHEAT?"true":"false")<<",\"ticks\":"<<ticks<<",\"writes\":"<<host.writes<<",\"sounds\":"<<effects.playCount()<<",\"uploads\":"<<renderer.uploads<<",\"draws\":"<<renderer.draws;
 if(game){auto& f=game->frame();o<<",\"width\":"<<f.logicalWidth<<",\"height\":"<<f.logicalHeight;auto* w=game->validationWorld();if(w)o<<",\"stage\":"<<w->stage()<<",\"time\":"<<w->time()<<",\"actors\":"<<w->actors().size()<<",\"paused\":"<<(w->paused()?"true":"false");}
 o<<"}";status=o.str();return status.c_str();}
EMSCRIPTEN_KEEPALIVE const char* md_ui(){std::ostringstream o;o<<"[";if(game){bool first=true;for(auto& s:game->frame().sprites){if(s.image.empty())continue;if(!first)o<<",";first=false;o<<"{\"image\":\""<<s.image<<"\",\"x\":"<<s.destination.x<<",\"y\":"<<s.destination.y<<",\"w\":"<<s.destination.width<<",\"h\":"<<s.destination.height<<"}";}}o<<"]";status=o.str();return status.c_str();}
EMSCRIPTEN_KEEPALIVE int md_command(unsigned c){return MD_WEB_CHEAT&&game&&game->applyValidationCommand(c);}
EMSCRIPTEN_KEEPALIVE float md_audio_probe(int start){if(!MD_WEB_CHEAT)return -1;if(start){effects.play("HMG01.wav",.5f);effects.play("BuildMachComplete.wav",.5f);}return effects.playbackTime("BuildMachComplete.wav");}
EMSCRIPTEN_KEEPALIVE const char* md_save_info(){std::ostringstream o;o<<"[";auto b=host.readSave();if(b){auto s=SaveData::decode(*b);for(size_t i=0;i<5;++i){if(i)o<<",";o<<"{\"active\":"<<(s.active(i)?"true":"false")<<",\"stage\":"<<s.stage(i)<<",\"credits\":"<<s.credits(i)<<",\"builds\":"<<s.builds(i).size()<<",\"attacks\":"<<s.attacks(i).size()<<"}";}}o<<"]";status=o.str();return status.c_str();}
}
int main(){return 0;}
