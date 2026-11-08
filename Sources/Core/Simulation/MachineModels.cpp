#include "Core/Simulation/Battle.hpp"
#include "Core/Contracts/Game.hpp"
#include "Core/Simulation/AircraftModels.hpp"
#include "Core/Simulation/AircraftVisual.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <stdexcept>

namespace md {
namespace {
AssemblyPart assembly(const Chunk& c) {
    if(c.type!=4 || c.data.size()<26) throw std::runtime_error("Invalid original assembly part");
    AssemblyPart p;p.name=c.text(0,24);p.parentSocket=c.byte(24);p.zOrder=c.byte(25);
    for(const auto& child:c.children) if(child.type==4) p.children.push_back(assembly(child));
    return p;
}
}
BattleModels BattleModels::parse(const Chunk& parts,const Chunk& machines) {
    BattleModels result;
    std::function<void(const Chunk&)> readPart=[&](const Chunk& c) {
        if(c.type==1&&!c.data.empty()) {
            if(c.data.size()<27) throw std::runtime_error("Truncated original part definition");
            PartDefinition p; p.name=c.text(0,24);p.type=c.byte(24);p.tileCount=c.byte(25);
            p.image=p.tileCount ? p.name+".png" : "PartsDummy.png";p.splitX=std::max(1u,unsigned(p.tileCount));
            auto count=c.byte(26);
            if(c.data.size()<27+std::size_t(count)*8) throw std::runtime_error("Truncated original part sockets");
            for(unsigned i=0;i<count;++i) p.sockets.push_back({c.real32(27+i*8),c.real32(31+i*8)});
            result.parts_.insert_or_assign(p.name,std::move(p));
        }
        for(const auto& child:c.children) readPart(child);
    };
    readPart(parts);
    std::function<void(const Chunk&)> readMachine=[&](const Chunk& c) {
        if(c.type==3&&!c.data.empty()) {
            std::vector<AssemblyPart> model;
            for(const auto& child:c.children) if(child.type==4) model.push_back(assembly(child));
            result.machines_.insert_or_assign(c.text(0,24),std::move(model));
        }
        for(const auto& child:c.children) readMachine(child);
    };
    readMachine(machines);return result;
}
void BattleModels::append(const Chunk& parts,const Chunk& machines) {
    auto extra=parse(parts,machines);
    for(const auto& [name,part]:extra.parts_) if(parts_.contains(name)) throw std::runtime_error("Duplicate additional part: "+name);
    for(const auto& [name,model]:extra.machines_) if(machines_.contains(name)) throw std::runtime_error("Duplicate additional machine: "+name);
    parts_.merge(extra.parts_);machines_.merge(extra.machines_);
}
BattleModels BattleModels::load(ResourceAccess& r) {
    auto result=parse(Chunk::parse(r.read("RawData/PartsInfo.mcb")),Chunk::parse(r.read("RawData/MachInfo.mcb")));
    result.append(Chunk::parse(r.read("RawData/Level10Parts.mcb")),Chunk::parse(r.read("RawData/Level10Machines.mcb")));
    auto table=[&](const char* name){auto bytes=r.read(std::string("Tables/")+name+".tsv");return DataTable::parse({reinterpret_cast<const char*>(bytes.data()),bytes.size()});};
    result.configureTiles(table("PartsList"),table("TileList"));
    appendAircraftModels(result);
    for(const char* name:{"AircraftLight","AircraftHeavy","AircraftDuct","AircraftTransport"})for(unsigned level=1;level<=10;++level){
        char suffix[8];std::snprintf(suffix,sizeof suffix,"_%02u",level);
        result.appendMachineVariant(std::string(name)+suffix,name,suffix);
    }
    return result;
}
void BattleModels::appendMachineVariant(std::string name,std::string_view source,std::string_view weaponSuffix){
    if(machines_.contains(name))throw std::runtime_error("Duplicate machine variant");
    const auto* original=machine(source);if(!original)throw std::runtime_error("Missing machine variant source");
    auto copy=*original;
    auto walk=[&](auto&& self,auto& nodes)->void{for(auto& node:nodes){
        const auto* p=part(node.name);if(!p)throw std::runtime_error("Missing variant part");
        if(p->type==4){auto alias=*p;alias.name+=weaponSuffix;node.name=alias.name;parts_.try_emplace(alias.name,std::move(alias));}
        self(self,node.children);
    }};walk(walk,copy);machines_.emplace(std::move(name),std::move(copy));
}
const PartDefinition* BattleModels::part(std::string_view name) const { auto i=parts_.find(std::string(name));return i==parts_.end()?nullptr:&i->second; }
const std::vector<AssemblyPart>* BattleModels::machine(std::string_view name) const { auto i=machines_.find(std::string(name));return i==machines_.end()?nullptr:&i->second; }
Vec2 PartDefinition::socket(unsigned index,float scale) const {
    if(index>=sockets.size()) return {};
    return {std::trunc(sockets[index].x)*scale,std::trunc(sockets[index].y)*scale};
}
void BattleModels::configureTiles(const DataTable& partsList,const DataTable& tilesList) {
    for(std::size_t row=0;row<partsList.rowCount();++row) {
        auto i=parts_.find(std::string(partsList.value(row,"PartsName")));
        if(i==parts_.end()) continue;
        auto tileName=partsList.value(row,"TileName");if(tileName.empty()||tileName=="-") continue;
        std::string image=std::string(tileName)+".png";
        auto tileRow=tilesList.findRow(image);if(!tileRow) throw std::runtime_error("Shared part tile missing from TileList");
        auto x=tilesList.integer(*tileRow,"X"),y=tilesList.integer(*tileRow,"Y"),tile=partsList.integer(row,"No");
        if(!x||!y||!tile||*x<1||*y<1||*tile<0) throw std::runtime_error("Invalid original tile grid");
        i->second.image=image;i->second.splitX=unsigned(*x);i->second.splitY=unsigned(*y);i->second.tileNo=unsigned(*tile);
    }
}

std::vector<PartPose> BattleModels::pose(std::string_view name,float scale,float bodyAngle,float weaponAngle,float radarAngle) const {
    auto tree=machine(name);if(!tree) throw std::runtime_error("Unknown original machine model");
    return pose(*tree,scale,bodyAngle,weaponAngle,radarAngle);
}
std::vector<PartPose> BattleModels::pose(const std::vector<AssemblyPart>& tree,float scale,float bodyAngle,float weaponAngle,float radarAngle) const {
    // Build the original pose first, then apply the documented aircraft size.
    std::vector<PartPose> poses;
    auto rotate=[](Vec2 v,float a){float c=std::cos(a),s=std::sin(a);return Vec2{v.x*c-v.y*s,v.x*s+v.y*c};};
    std::function<void(const AssemblyPart&,Vec2,float,bool)> append=[&](const AssemblyPart& a,Vec2 basePos,float parentAngle,bool hasParent){
        auto p=part(a.name);if(!p) throw std::runtime_error("Assembly references unknown part");
        float angle=a.angle.value_or(p->type==4?weaponAngle:(p->type==7?radarAngle:parentAngle));
        Vec2 anchor=p->socket(0,scale),drawingAnchor=anchor;
        if(a.reverse) drawingAnchor.y=-drawingAnchor.y;
        Vec2 baseRot=rotate(basePos,parentAngle),anchorRot=rotate(drawingAnchor,angle),add=rotate(a.addPosition,angle);
        poses.push_back({p,{baseRot.x+add.x-anchorRot.x,baseRot.y+add.y-anchorRot.y},angle,scale,a.zOrder,a.reverse,a.visualScale,hasParent});
        for(const auto& child:a.children) {
            Vec2 socket=p->socket(child.parentSocket,scale);
            append(child,{basePos.x+socket.x-anchor.x,basePos.y+socket.y-anchor.y},angle,true);
        }
    };
    for(const auto& a:tree) append(a,{},bodyAngle,false);
    const float assemblyScale=tree.empty()?1.f:aircraftAssemblyScale(tree.front().name);
    for(auto& p:poses){
        p.localCenter.x*=assemblyScale;p.localCenter.y*=assemblyScale;
        p.deviceScale*=assemblyScale;
    }
    std::stable_sort(poses.begin(),poses.end(),[](const auto& a,const auto& b){return a.zOrder<b.zOrder;});
    return poses;
}
std::optional<Vec2> PartPose::muzzle(unsigned index,bool reverse) const {
    if(!definition || definition->sockets.size()<2 || index>=definition->sockets.size()) return std::nullopt;
    Vec2 base=definition->socket(0,deviceScale),socket=definition->socket(index,deviceScale);
    Vec2 relative{socket.x-base.x,(reverse||this->reverse)?base.y-socket.y:socket.y-base.y};
    float c=std::cos(angle),s=std::sin(angle);
    return Vec2{localCenter.x+relative.x*c-relative.y*s,localCenter.y+relative.x*s+relative.y*c};
}
unsigned PartPose::nextMuzzle(unsigned current) const {
    return !definition || current+1>=definition->sockets.size()?1:current+1;
}

MachineDefinition MachineDefinition::fromTable(const DataTable& table,std::size_t row,BattleGeometry geometry) {
    auto integer=[&](std::string_view key){auto v=table.integer(row,key);if(!v) throw std::runtime_error("Invalid MachList integer");return int(*v);};
    MachineDefinition m;m.name=table.value(row,"MachName");m.hp=integer("HP")*2;m.experience=integer("EXP");
    auto speed=table.number(row,"Spd");if(!speed) throw std::runtime_error("Invalid MachList speed");
    m.speed=*speed*geometry.deviceScale;m.radius=int(float(integer("Rad"))*geometry.deviceScale);
    auto type=table.value(row,"Type");
    if(type=="TANK") m.type=1;else if(type=="PLANE") m.type=2;else if(type=="TURRET") m.type=3;else if(type=="BOT") m.type=5;
    auto ref=table.value(row,"Refer");
    if(ref=="BOT_D") m.referenceType=1;else if(ref=="BOT_P") m.referenceType=2;else if(ref=="BOT_R") m.referenceType=3;
    return m;
}

}
