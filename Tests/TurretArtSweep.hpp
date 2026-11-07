#pragma once
#include "Core/Simulation/World.hpp"
#include <array>
#include <cmath>
#include <stdexcept>

namespace md::validation {
// Uses real resource/model assembly and the platform GPU renderer. No save,
// game session or gameplay state is changed to display the comparison frame.
inline Frame turretArtFrame(ResourceAccess& resources, unsigned family) {
    constexpr std::array families{"Gun","Msl","MG","Ion","Lsr","Pla"};
    constexpr std::array images{"gun","msl","mg","ion","lsr","pla"};
    auto require=[](bool ok,const char* message){if(!ok)throw std::runtime_error(message);};
    require(family<families.size(),"Art family out of range");
    auto models=BattleModels::load(resources);
    auto size=[&](std::string_view n){return resources.imageSize(n);};
    Frame frame;frame.logicalWidth=800;frame.logicalHeight=640;frame.background={.18f,.19f,.17f,1};
    const std::string original=std::string("Turret_")+images[family]+"_05.png";
    const auto originalBytes=resources.read("Images/"+original);
    for(unsigned direction=0;direction<4;++direction)for(unsigned level=5;level<=10;++level){
        auto suffix=std::string(level<10?"_0":"_")+std::to_string(level);
        auto machine=std::string("Tur_")+families[family];
        auto angle=direction*1.57079632679f;
        auto pose=models.pose(machine+suffix,1,0,angle,angle);
        auto old=models.pose(machine+"_05",1,0,angle,angle);
        require(pose.size()==old.size(),"Art part count changed");
        unsigned bodies=0,weapons=0;
        for(unsigned i=0;i<pose.size();++i){
            const auto& a=pose[i];const auto& b=old[i];
            require(a.localCenter.x==b.localCenter.x&&a.localCenter.y==b.localCenter.y&&a.angle==b.angle&&a.visualScale==b.visualScale&&a.reverse==b.reverse,"Art pose changed");
            require(a.definition->sockets.size()==b.definition->sockets.size(),"Art socket count changed");
            for(unsigned socket=1;socket<a.definition->sockets.size();++socket){
                auto x=a.muzzle(socket),y=b.muzzle(socket);
                require(x&&y&&x->x==y->x&&x->y==y->y,"Art muzzle changed");
            }
            if(!a.definition->tileCount)continue;
            auto x=size(a.definition->image),y=size(b.definition->image);
            require(x.x>0&&x.y>0&&x.x==y.x&&x.y==y.y,"Art canvas changed");
            if(a.definition->image.starts_with("Turret_b_")){
                require(a.definition->image=="Turret_b"+suffix+".png","Wrong body level");++bodies;
            }else if(a.definition->image.starts_with(std::string("Turret_")+images[family]+"_")){
                require(resources.read("Images/"+a.definition->image)==originalBytes,"Weapon image differs from original level five");++weapons;
            }
        }
        require(bodies==1&&weapons==1,"Missing turret body or weapon");
        models.appendSprites(frame,pose,{80.f+128.f*(level-5),80.f+160.f*direction},size);
    }
    return frame;
}
}
