#include "Core/Simulation/Combat.hpp"
#include <array>
#include <cmath>
#include <stdexcept>

namespace md {
int WeaponDefinition::attackPoint() const {
    const float interval=float(shotCount)*shotDelay+reloadDelay;
    if(!(interval>0))throw std::runtime_error("Weapon attack interval must be positive");
    return int((1.f/interval)*float(int(damage)*int(shotCount))+.5f);
}
WeaponDefinition WeaponDefinition::fromTable(const DataTable& t,std::size_t row,bool phone){
    auto integer=[&](std::string_view field){auto v=t.integer(row,field);if(!v)throw std::runtime_error("Missing weapon integer: "+std::string(field));return *v;};
    auto number=[&](std::string_view field){auto v=t.number(row,field);if(!v||!std::isfinite(*v))throw std::runtime_error("Missing weapon number: "+std::string(field));return *v;};
    WeaponDefinition w;w.name=t.value(row,"Parts");
    constexpr std::array names{"Gun","Shotgun","Laser","Railgun","EMP","Bomb","Missile","Sniper","Nuclear","Bullet"};
    const auto type=t.value(row,"Type");bool found=false;
    for(std::size_t i=0;i<names.size();++i)if(type==names[i]){w.type=WeaponType(i);found=true;break;}
    if(!found)throw std::runtime_error("Unknown original weapon type");
    w.damage=std::int16_t(integer("Dmg"));w.damageRange=std::int16_t(integer("Range"));
    // GlobalInfo::openWeaponInfo 0x1deac–0x1e12a applies 1.5 before
    // truncation, then the phone scale before a second truncation.
    int range=int(float(integer("ShtRng"))*1.5f);
    w.speed=number("Speed");
    if(phone){range=int(float(range)*.5f);w.damageRange=std::int16_t(float(w.damageRange)*.5f);w.speed*=.5f;}
    w.shotRange=std::int16_t(range);w.shotCount=std::uint8_t(integer("ShtCnt"));
    w.shotDelay=number("ShtDly");w.reloadDelay=number("RldDly");w.aimRate=number("AimRt");
    w.special1=number("sp1");w.special2=number("sp2");w.shell=std::int16_t(integer("Shell"));w.uiTile=std::uint8_t(integer("UiTile"));
    const auto bullet=t.value(row,"BltTile");
    if(bullet!="-"){
        auto split=bullet.find('_');if(split==std::string_view::npos)throw std::runtime_error("Invalid original bullet tile");
        w.bulletImage="Blt_"+std::string(bullet.substr(0,split))+".png";w.bulletTile=std::stoi(std::string(bullet.substr(split+1)));
    }
    const auto effect=t.value(row,"EffType");
    constexpr std::array effectNames{"-","unused","Flame","Ion","Smoke","Trail","Laser","Rail","Shotgun"};
    for(std::size_t i=0;i<effectNames.size();++i)if(effect==effectNames[i])w.effectType=int(i);
    if(w.effectType){w.effectImage=std::string(t.value(row,"EffTile"))+".png";if(w.effectType==7)w.effectColumns=w.effectRows=4;}
    const auto explosion=t.value(row,"ExplodeEff");
    if(explosion!="-"){w.explodeImage="Efx_"+std::string(explosion)+".png";w.explodeScale=number("ExplScl");}
    if(t.value(row,"FireSound")!="-")w.fireSound=t.value(row,"FireSound");
    return w;
}
WeaponCatalog WeaponCatalog::load(const DataTable& t,bool phone){
    WeaponCatalog result;
    for(std::size_t i=0;i<t.rowCount();++i){auto w=WeaponDefinition::fromTable(t,i,phone);result.definitions_.insert_or_assign(w.name,std::move(w));}
    return result;
}
const WeaponDefinition* WeaponCatalog::find(std::string_view name)const {auto i=definitions_.find(std::string(name));return i==definitions_.end()?nullptr:&i->second;}
}
