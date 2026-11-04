#include "SummaryDialog.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace md;
struct Resources:ResourceAccess{
 std::filesystem::path root;std::map<std::string,Vec2> sizes;explicit Resources(std::filesystem::path p):root(std::move(p)){}
 Bytes read(std::string_view p)override{std::ifstream f(root/std::string(p),std::ios::binary);assert(f);return {std::istreambuf_iterator<char>(f),{}};}
 Vec2 imageSize(std::string_view n)override{auto key=std::string(n);if(sizes.contains(key))return sizes.at(key);auto b=read("Images/"+key);auto u=[&](int i){return std::uint32_t(b[i])<<24|std::uint32_t(b[i+1])<<16|std::uint32_t(b[i+2])<<8|b[i+3];};return sizes[key]={float(u(16)),float(u(20))};}
};
void tap(SummaryDialog& d,float x,float y){d.touch({1,TouchPhase::began,{x,y}});d.touch({1,TouchPhase::ended,{x,y}});}
bool has(const Frame& f,std::string_view image){for(const auto& s:f.sprites)if(s.image==image)return true;return false;}
int main(int argc,char** argv){
 Resources r(argc>1?argv[1]:"Resources");std::vector<Vec2> rewards;
 SummaryDialog d(r,[&](Frame&,const StageReward&,Vec2 p,float scale){assert(scale==.5f);rewards.push_back(p);});
 SummaryState s;assert(!d.touch({1,TouchPhase::began,{0,0}}));s.visible=true;s.cleared=true;s.mission=2;s.elapsedSeconds=125;s.kills=30;s.deaths=5;s.reward.credits=550;s.reward.attack="AirStrike-Bomb";s.reward.attackCount=3;d.setState(s);
 Frame f;d.append(f);assert(has(f,"CommonUI_StageClear.png")&&has(f,"SummaryBtn_NextStage.png"));assert(rewards.size()==3&&rewards[0].x==90&&rewards[1].x==130&&rewards[2].x==170);
 for(auto& sp:f.sprites){assert(std::filesystem::exists(sp.image.starts_with("Generated/")?r.root/sp.image:r.root/"Images"/sp.image));if(sp.sourcePx&&!sp.image.starts_with("Generated/")){auto sz=r.imageSize(sp.image);assert(sp.sourcePx->x+sp.sourcePx->width<=sz.x&&sp.sourcePx->y+sp.sourcePx->height<=sz.y);}}
 tap(d,198,268);assert(d.takeActions().at(0).buttonId==0x3f9);tap(d,134,268);assert(d.takeActions().at(0).buttonId==0x3f8);tap(d,70,268);assert(d.takeActions().at(0).buttonId==0x41a);
 d.touch({2,TouchPhase::began,{198,268}});d.setState(s);d.touch({2,TouchPhase::ended,{198,268}});assert(d.takeActions().at(0).buttonId==0x3f9);
 s.cleared=false;d.setState(s);f={};d.append(f);assert(has(f,"CommonUI_Top.png")&&!has(f,"CommonUI_StageClear.png"));assert(has(f,"SummaryBtn_Retry.png"));tap(d,198,316);assert(d.takeActions().at(0).buttonId==0x3fa);
 s.cleared=true;s.mission=79;d.setState(s);f={};d.append(f);assert(has(f,"SummaryBtn_Retry.png"));
 s.mission=0;s.reward.attack.clear();s.reward.build="MachineGun";d.setState(s);rewards.clear();f={};d.append(f);assert(rewards.size()==1&&rewards[0].x==90);
 s.visible=false;d.setState(s);assert(!d.touch({9,TouchPhase::began,{130,200}}));
 std::cout<<"SummaryDialog original clear/fail/final branches, rewards and buttons passed\n";
}
