#include "Core/Simulation/Loot.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace md {
namespace {
float randomFloat(DarwinRandom& random) { return static_cast<float>(random.next())*0x1p-31f; }
float randomAngle(DarwinRandom& random) {const float a=randomFloat(random)*3.1415927410125732421875f;return a+a;}
void ease(float& value) {if(std::abs(value)<=.01f)value=0;else value+=(0-value)/7.f;}
}
LootDefinition LootDefinition::fromTable(const DataTable& table,std::size_t row) {
    LootDefinition d;d.code=table.value(row,"Code");d.name=table.value(row,"Name");
    const auto icon=table.integer(row,"Icon");if(!icon)throw std::runtime_error("Invalid original loot icon");d.icon=static_cast<int>(*icon);
    const auto type=table.value(row,"Type");if(type=="Coin")d.type=LootType::coin;else if(type=="Mineral")d.type=LootType::mineral;
    return d;
}
float LootPickup::applyCells(float cells,float capacity) const {return type==LootType::mineral?std::min(capacity,cells+static_cast<float>(value)):cells;}
LootState LootState::create(LootDefinition definition,int value,Vec2 position,double time,float scale,DarwinRandom& random) {
    LootState item;item.definition=std::move(definition);item.value=value;item.position=position;
    const float iconRandom=randomFloat(random);item.iconAngle=(iconRandom+iconRandom)-1.f;
    const float angle=randomAngle(random),velocity=randomFloat(random)*4.f+3.f;
    item.velocity={static_cast<float>(std::cos(static_cast<double>(angle))*static_cast<double>(velocity)),
                   static_cast<float>(std::sin(static_cast<double>(angle))*static_cast<double>(velocity))};
    item.checkLength=100.f*scale;item.maximumCheckLength=400.f*scale;item.checkIncrement=10.f*scale;
    item.createTime=static_cast<float>(time);return item;
}
std::optional<LootPickup> LootState::tick(double time,std::span<const LootCollector> collectors,DarwinRandom& random) {
    if(removed)return std::nullopt;
    std::optional<LootPickup> pickup;
    auto target=collectors.end();
    if(picker) target=std::find_if(collectors.begin(),collectors.end(),[&](const auto& unit){return unit.id==*picker;});
    if(picker&&target==collectors.end())picker.reset(); // same effect as pickerDead before removal.
    if(picker) {
        const Vec2 delta{target->position.x-position.x,target->position.y-position.y};
        velocity.x+=(delta.x*.05f-velocity.x)/10.f;velocity.y+=(delta.y*.05f-velocity.y)/10.f;
        speed+=(.1f-speed)/100.f;position.x+=velocity.x;position.y+=velocity.y;
        if(std::abs(delta.x)<20.f&&std::abs(delta.y)<20.f) {
            std::string label=definition.name;
            if(definition.type==LootType::coin)label=std::to_string(value)+"cr";
            else if(definition.type==LootType::mineral)label=std::to_string(value)+"cell";
            pickup=LootPickup{*picker,definition.type,value,std::move(label),3};removed=true;
        }
    } else {ease(velocity.x);ease(velocity.y);position.x+=velocity.x;position.y+=velocity.y;}
    if(!removed&&!picker&&time-static_cast<double>(checkTime)>.5) {
        if(checkLength<maximumCheckLength)checkLength+=checkIncrement;
        for(const auto& unit:collectors) {
            if(unit.machineType!=0||unit.hp<=0)continue;
            if(std::abs(unit.position.x-position.x)<checkLength&&std::abs(unit.position.y-position.y)<checkLength) {picker=unit.id;break;}
        }
        if(!picker&&time-static_cast<double>(createTime)>180.0)removed=true;
        checkTime=static_cast<float>(time);
    }
    // Original still updates appearance on the tick which picked up/expired.
    glowAngle=randomAngle(random);
    glowScale=std::sin(static_cast<float>(time*10.0))*.01f+.5f;
    glowAlpha=randomFloat(random)*.8f+.2f;
    return pickup;
}
void LootState::appendSprites(Frame& frame,Vec2 screen,const std::function<Vec2(std::string_view)>& imageSize,float retinaScale) const {
    if(removed)return;
    auto append=[&](std::string name,unsigned columns,unsigned rows,int tile,float scale,float angle,float alpha,Blend blend) {
        const Vec2 size=imageSize(name);if(size.x<=0||size.y<=0)return;
        const unsigned selected=std::min(static_cast<unsigned>(std::max(0,tile)),columns*rows-1);
        const float width=size.x/columns,height=size.y/rows,w=width*scale*retinaScale,h=height*scale*retinaScale;
        Sprite s;s.image=std::move(name);s.sourcePx=Rect{float(selected%columns)*width,float(selected/columns)*height,width,height};
        s.destination={screen.x-w*.5f,screen.y-h*.5f,w,h};s.rotationDegrees=-angle*180.f/3.14159265358979323846f;
        s.color={1,1,1,alpha};s.blend=blend;frame.sprites.push_back(std::move(s));
    };
    append("Efx_Item2.png",1,1,0,glowScale,glowAngle,glowAlpha,Blend::additive);
    append("Icon_"+std::to_string(definition.icon/1000)+".png",4,4,definition.icon%1000-1,
           definition.type==LootType::other?.5f:1.f,iconAngle,1,Blend::alpha);
}
std::vector<LootState> enemyLootDrops(int coin,int rate,float factor,Vec2 position,double time,float scale,DarwinRandom& random,
    const std::function<const LootDefinition*(std::string_view)>& lookup) {
    std::vector<LootState> drops;
    if(static_cast<int>(random.next()%100)>=rate)return drops;
    auto emit=[&](bool mineral,int value) {
        const int tier=value<50?1:(value<500?2:(value<1000?3:(value<2000?4:5)));
        const std::string code=(mineral?"0010":"0000")+std::to_string(tier);
        const auto* info=lookup(code);if(!info)throw std::runtime_error("Missing original drop item: "+code);
        drops.push_back(LootState::create(*info,value,position,time,scale,random));
    };
    auto amount=[&](){return static_cast<float>(coin)+randomFloat(random)*(static_cast<float>(coin)/3.f);};
    emit(true,static_cast<int>(amount()));
    if((random.next()&1)==0)emit(false,static_cast<int>(amount()*factor));
    return drops;
}
}
