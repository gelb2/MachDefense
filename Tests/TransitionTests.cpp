#include "Transition.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace md;
namespace {
struct Resources:ResourceAccess{
    std::filesystem::path root;explicit Resources(std::filesystem::path p):root(std::move(p)){}
    Bytes read(std::string_view n)override{std::ifstream f(root/std::string(n),std::ios::binary);assert(f);return {std::istreambuf_iterator<char>(f),{}};}
    Vec2 imageSize(std::string_view n)override{
        auto b=read("Images/"+std::string(n));assert(b.size()>=24&&b[0]==137);
        auto u=[&](int a){return std::uint32_t(b[a])<<24|std::uint32_t(b[a+1])<<16|std::uint32_t(b[a+2])<<8|b[a+3];};
        return {float(u(16)),float(u(20))};
    }
};
struct Host:HostServices{
    std::vector<std::string> sounds;
    std::optional<Bytes>readSave()override{return {};}
    bool writeSave(std::span<const std::uint8_t>)override{assert(false);return false;}
    void playSound(std::string_view n,float,float)override{sounds.emplace_back(n);}
    void playMusic(std::string_view,float)override{}
    void stopMusic()override{}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{assert(false);}
};
bool near(float a,float b){return std::fabs(a-b)<.0001f;}
}
int main(int argc,char**argv){
    Resources resources(argc>1?argv[1]:"Resources");Host host;Transition tr(resources,host);
    Frame frame;tr.append(frame);assert(frame.sprites.empty());assert(tr.state()==8);
    assert(!tr.request(0));assert(!tr.request(6));assert(tr.request(2));assert(!tr.request(3));
    int tick=0,refreshTick=0,refreshCount=0;std::map<unsigned,int> first;
    while(tick<200){
        ++tick;const auto refresh=tr.tick();first.try_emplace(tr.state(),tick);
        if(tick==1){assert(tr.positions()[0].y==672);assert(tr.positions()[1].y==-192);assert(tr.positions()[2].x==-128);assert(tr.positions()[3].x==448);}
        if(tick==2){assert(near(tr.positions()[2].x,-83.2f));assert(near(tr.positions()[3].x,403.2f));}
        if(refresh){++refreshCount;refreshTick=tick;assert(*refresh==2);assert(tr.positions()[0].y==304);assert(tr.positions()[1].y==176);assert(tr.centerScale()==1);}
        frame.sprites.clear();tr.append(frame);
        if(tr.visible()){
            assert(frame.sprites.size()==5);
            assert(frame.sprites[0].image=="Loading_Left.png");assert(frame.sprites[3].image=="Loading_Bottom.png");
            assert(frame.sprites[0].destination.width==256);assert(frame.sprites[0].destination.height==512);
            assert(frame.sprites[2].destination.width==384);assert(frame.sprites[2].destination.height==384);
            const auto& center=frame.sprites[4].destination;
            assert(near(center.x+center.width/2,160));
            assert(near(center.y+center.height/2,480-(tr.positions()[1].y+84)));
        }
        if(tr.state()==8)break;
    }
    assert(refreshCount==1);assert(first[2]==13);assert(first[3]==60);assert(first[4]==69);assert(refreshTick==70);assert(first[7]==78);assert(tick==157);
    assert(host.sounds==std::vector<std::string>({"Loading_Close.wav","Loading_Close.wav","Loading_Open.wav"}));
    assert(frame.sprites.empty());assert(tr.canChangeScreen());
    assert(tr.request(3));while(tr.state()!=7)tr.tick();assert(tr.request(1)); // Original permits a new request during state 7.
    assert(!tr.tick());assert(tr.state()==1);
    Host padHost;Transition pad(resources,padHost,true);assert(pad.request(3));pad.tick();
    Frame padFrame;padFrame.logicalWidth=768;padFrame.logicalHeight=1024;pad.append(padFrame);
    assert(pad.positions()[0].x==384&&pad.positions()[0].y==1408);assert(pad.positions()[1].y==-384);
    assert(pad.positions()[2].x==-256&&pad.positions()[3].x==1024);
    assert(padFrame.sprites[0].destination.width==512&&padFrame.sprites[0].destination.height==1024);
    std::cout<<"PASS transition: original assets/geometry, exact 157-tick phone sequence, covered refresh at tick 70, sound order, iPad values, re-entry gate\n";
}
