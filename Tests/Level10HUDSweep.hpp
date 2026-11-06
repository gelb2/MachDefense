#pragma once
#include "Core/Session/GameSession.hpp"
#include "Core/Simulation/World.hpp"
#include "Core/Contracts/FrameLayout.hpp"
#include "Core/Presentation/SpriteFont.hpp"
#include "../Modules/SaveTransfer/Validate.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <set>
#include <sstream>
#include <stdexcept>

namespace md::validation {
// Test-owned save storage. The optional output receives sound only; the sweep
// never opens or writes the player's save/IndexedDB container.
struct Level10SweepHost final:HostServices {
    Bytes saved;unsigned writes=0;std::unordered_map<std::string,unsigned> soundCalls;HostServices* output=nullptr;
    explicit Level10SweepHost(HostServices* sound=nullptr):output(sound){}
    std::optional<Bytes> readSave()override{return saved.empty()?std::nullopt:std::optional(saved);}
    bool writeSave(std::span<const std::uint8_t> bytes)override{saved.assign(bytes.begin(),bytes.end());++writes;return true;}
    void playSound(std::string_view n,float gain,float pan)override{++soundCalls[std::string(n)];if(output)output->playSound(n,gain,pan);}
    void playSoundAt(std::string_view n,Vec2 p,float gain)override{++soundCalls[std::string(n)];if(output)output->playSoundAt(n,p,gain);}
    void playMusic(std::string_view,float)override{}
    void stopMusic()override{}
    void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{throw std::runtime_error("Unexpected sweep name request");}
};
inline std::string runLevel10HUDSweep(ResourceAccess& resources,HostServices* audioOutput=nullptr,
                                    const std::function<void(const Frame&,std::string_view)>& inspect={}) {
    auto require=[](bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);};
    auto table=[&](const char* name){auto b=resources.read(std::string("Tables/")+name+".tsv");return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});};
    auto ticks=[](GameSession& g,int n=160){while(n--)g.tick();};
    auto tap=[](GameSession& g,Vec2 p){g.touch({87,TouchPhase::began,p});g.touch({87,TouchPhase::ended,p});};
    auto press=[&](GameSession& g,std::string_view image){
        for(const auto& s:g.frame().sprites)if(s.image==image){auto r=s.destination;tap(g,{r.x+r.width/2,r.y+r.height/2});return;}
        throw std::runtime_error("Missing action image "+std::string(image));
    };
    auto checkImages=[&](const Frame& frame){for(const auto& s:frame.sprites){
        require(std::isfinite(s.destination.x)&&std::isfinite(s.destination.y),"Non-finite UI geometry");
        if(s.image.empty())continue;auto size=resources.imageSize(s.image);
        require(size.x>0&&size.y>0,"Missing rendered image "+s.image);
        if(s.sourcePx)require(s.sourcePx->x>=0&&s.sourcePx->y>=0&&s.sourcePx->x+s.sourcePx->width<=size.x+.01f&&s.sourcePx->y+s.sourcePx->height<=size.y+.01f,"Image crop "+s.image);
    }};
    auto seed=[](){auto s=SaveData::empty(123);s.createProfile("LEVEL10",1);s.writeWord(SaveData::slotStart+0xc,5);s.setCredits(0,10000000);return s;};
    auto enter=[&](GameSession& g,const FrameLayout& layout){const auto c=layout.center(),br=layout.bottomRight();tap(g,{160+c.x,240+c.y});ticks(g);tap(g,{160+c.x,140+c.y});ticks(g);tap(g,{240+br.x,450+br.y});ticks(g);require(g.validationWorld()!=nullptr,"Sweep world did not start");};
    const auto base=table("BaseUpgrade"),named=table("MachBuild_Name"),parts=table("MachBuild_Parts"),special=table("SpAttackList");
    const std::array<std::string,6> turretNames{"RifleTurret","MissileTurret","Vulcan","IonCannon","LaserTurret","PlasmaCannon"};
    const std::array<std::string,5> attackNames{"AirStrike-Bomb","AirStrike-Missile","AirStrike-Nuclear","AirStrike-EMP","Crossfire-Missile"};
    auto full=[](std::string_view name,unsigned level){char suffix[16];std::snprintf(suffix,sizeof suffix,"_%02u",level+1);return std::string(name)+suffix;};
    struct Case{unsigned kind,category,level;std::string name;std::int64_t cost;};std::vector<Case> cases;
    BaseUpgradeCatalog bases(base);AttackCatalog attacks(special);const auto builds=BuildCatalog::load(parts,named);
    for(unsigned category=0;category<2;++category){require(bases.maximumLevel(BaseUpgrade(category))==9,"Base cap not ten");for(unsigned level=0;level<10;++level)cases.push_back({0,category,level,category?"BaseDefense":"BaseCannon",bases.level(BaseUpgrade(category),level).cost});}
    for(const auto& name:turretNames)for(unsigned level=0;level<10;++level){const auto* d=builds.find(full(name,level));require(d!=nullptr,"Missing level "+full(name,level));require(!builds.find(full(name,10)),"Turret exceeds ten "+name);cases.push_back({1,0,level,name,d->upgradeCost});}
    for(const auto& name:attackNames){require(attacks.maximumLevel(name)==10,"Special cap not ten "+name);for(unsigned level=0;level<10;++level)cases.push_back({2,0,level,name,attacks.upgradeCost({name,level,2,true})});}
    require(cases.size()==130,"Upgrade coverage must be 130 rows");
    const auto weaponTable=table("WeaponInfo");const auto weaponCatalog=WeaponCatalog::load(weaponTable,true);
    const std::array<std::string,6> prefixes{"Turret_gun_","Turret_msl_","Turret_mg_","Turret_ion_","Turret_lsr_","Turret_pla_"};
    auto metrics=resources.read("Generated/TrebuchetMS-Bold-48.tsv");
    SpriteFont font({reinterpret_cast<const char*>(metrics.data()),metrics.size()});
    unsigned statFrames=0;
    auto checkStats=[&](GameSession& game,const Case& item,unsigned level){
        if(item.kind!=1)return;
        auto family=std::find(turretNames.begin(),turretNames.end(),item.name)-turretNames.begin();
        auto weapon=[&](unsigned l){return weaponCatalog.find(prefixes[family]+(l<9?"0":"")+std::to_string(l+1));};
        const auto* current=weapon(level);require(current!=nullptr,"Missing UI weapon");
        const auto* next=level<9?weapon(level+1):nullptr;
        const auto* build=builds.find(full(item.name,level));
        const auto machines=table("MachList");
        auto hp=[&](const auto* d){return MachineDefinition::fromTable(machines,*machines.findRow(d->machine),BattleGeometry::phone()).hp;};
        const std::array values{current->attackPoint(),int(current->shotRange),hp(build)};
        const std::array nextValues{next?next->attackPoint():0,next?int(next->shotRange):0,next?hp(builds.find(full(item.name,level+1))):0};
        const std::array labels{"ATK.","Range","DEF."};
        for(unsigned n=0;n<3;++n){
            std::string text=std::string(labels[n])+" : "+std::to_string(values[n]);
            if(next)text+="(+"+std::to_string(nextValues[n]-values[n])+")";
            Frame expected;font.draw(expected,text,{n==1?102.f:32.f,n<2?295.75f:303.75f},6);
            for(const auto& glyph:expected.sprites){
                bool found=false;
                for(const auto& actual:game.frame().sprites)if(actual.image==glyph.image&&actual.sourcePx&&glyph.sourcePx&&actual.sourcePx->x==glyph.sourcePx->x&&actual.sourcePx->y==glyph.sourcePx->y&&std::abs(actual.destination.x-glyph.destination.x)<.01f&&std::abs(actual.destination.y-glyph.destination.y)<.01f){found=true;break;}
                require(found,item.name+" Lv."+std::to_string(level+1)+" UI "+text);
            }
        }
        ++statFrames;
    };
    unsigned bought=0,rejected=0,reloads=0,frames=0,hudConditions=0;
    for(const auto& item:cases)for(bool poor:{true,false}){
        const std::string context=item.name+" Lv."+std::to_string(item.level+1)+(poor?" poor":" funded");
        auto initial=seed();initial.setCredits(0,poor?std::max<std::int64_t>(0,item.cost-1):10000000);
        if(item.kind==0)initial.setBaseUpgrade(0,item.category,item.level);
        if(item.kind==1)initial.setBuilds(0,std::vector<OwnedBuild>{{item.name,item.level,true}});
        if(item.kind==2)initial.setAttacks(0,std::vector<OwnedAttack>{{item.name,item.level,2,true}});
        Level10SweepHost host(audioOutput);host.saved=initial.encode();GameSession game(resources,host,true);FrameLayout layout;enter(game,layout);
        if(item.kind==0){tap(game,{284,469});ticks(game,60);press(game,"Btn_BaseUpgrade_"+item.name+".png");}
        else{if(item.kind==2)tap(game,{160,445});tap(game,{294,403});tap(game,{item.kind==1?32.f:60.f,70.25f});}
        const auto before=*game.validationSave();const auto writes=host.writes;
        checkImages(game.frame());checkStats(game,item,item.level);++frames;
        if(item.kind==0)tap(game,{132,456});else tap(game,{item.kind==1?198.f:226.f,296.75f});
        ticks(game,2);const auto after=*game.validationSave();const bool accepted=!poor&&item.level<9;
        require(after.credits(0)==before.credits(0)-(accepted?item.cost:0),context+" debit");
        const auto level=item.kind==0?after.baseUpgrade(0,item.category):item.kind==1?after.builds(0).front().level:after.attacks(0).front().level;
        require(level==item.level+unsigned(accepted),context+" resulting level");require(host.writes==writes,context+" wrote during purchase");
        if(accepted)++bought;else ++rejected;
        checkImages(game.frame());checkStats(game,item,level);++frames;
        if(!poor){
            if(inspect)inspect(game.frame(),context);
            press(game,item.kind==0?"Btn_UpgradeBase_Close.png":"Common_Btn_CLOSE.png");ticks(game,60);
            tap(game,{284,435});tap(game,{130,120});ticks(game);
            require(host.writes==writes+1,context+" save-and-exit did not write exactly once");
            const auto disk=SaveData::decode(host.saved);require(disk.credits(0)==after.credits(0),context+" saved credits");
            const auto diskLevel=item.kind==0?disk.baseUpgrade(0,item.category):item.kind==1?disk.builds(0).front().level:disk.attacks(0).front().level;
            require(diskLevel==level,context+" saved level");
            const auto decoded=transfer::decode(transfer::encode(disk));transfer::validate(decoded,resources);
            require(decoded.encode()==disk.encode(),context+" recovery changed bytes");
            GameSession restored(resources,host,true);require(restored.validationSave()->encode()==disk.encode(),context+" session reload");++reloads;
        }
    }
    // The real equipment dialog retains explicit install/uninstall. Start with
    // seven installed and exercise an eighth before inspecting the full list.
    std::vector<OwnedBuild> owned;std::set<std::string> names;
    for(const auto* t:{&parts,&named})for(std::size_t row=0;row<t->rowCount();++row){const auto name=std::string(t->value(row,"MachName"));const auto cut=name.rfind('_');const auto stem=name.substr(0,cut);if(names.insert(stem).second)owned.push_back({stem,0,owned.size()<7});}
    require(owned.size()>8,"Insufficient production rows for HUD sweep");
    for(const Vec2 size:std::array<Vec2,3>{{{320,480},{420,912},{1024,1366}}}){
        auto initial=seed();initial.setBuilds(0,owned);Level10SweepHost host(audioOutput);host.saved=initial.encode();GameSession game(resources,host,true);FrameLayout layout;layout.resize(size.x,size.y);game.resize(size.x,size.y);enter(game,layout);
        const auto c=layout.center(),br=layout.bottomRight(),tr=layout.topRight();
        tap(game,{294+br.x,403+br.y});tap(game,{32+46*2+c.x,70.25f+62+c.y}); // eighth card
        press(game,"UpgradeBtn_Install.png");require(game.validationSave()->builds(0)[7].onSlot,"Eighth INSTALL rejected");
        press(game,"UpgradeBtn_Uninstall.png");require(!game.validationSave()->builds(0)[7].onSlot,"UNINSTALL ignored");
        press(game,"UpgradeBtn_Install.png");require(game.validationSave()->builds(0)[7].onSlot,"Reinstall rejected");
        // Install the remaining units through each original dialog button.
        for(std::size_t i=8;i<owned.size();++i){tap(game,{32+46.f*(i%5)+c.x,70.25f+62.f*(i/5)+c.y});press(game,"UpgradeBtn_Install.png");require(game.validationSave()->builds(0)[i].onSlot,"Remaining INSTALL rejected");}
        press(game,"Common_Btn_CLOSE.png");auto* world=game.validationWorld();world->setValidationCells(10000);game.tick();
        world->setValidationCells(0);game.tick();
        const auto saves=host.writes;const auto stateBefore=game.validationSave()->encode();const auto count=world->productionRows().size();
        const Vec2 down{292+tr.x,350+br.y},up{292+tr.x,65+tr.y};
        for(unsigned gesture=0;gesture<8;++gesture){game.touch({87,TouchPhase::began,down});game.tick();for(unsigned step=1;step<=12;++step){game.touch({87,TouchPhase::moved,{down.x,down.y+(up.y-down.y)*float(step)/12}});game.tick();}game.touch({87,TouchPhase::ended,up});}
        require(world->productionRows().size()==count,"HUD drag produced a unit");require(host.writes==saves,"HUD drag wrote save");require(game.validationSave()->encode()==stateBefore,"HUD drag modified progress");
        checkImages(game.frame());++frames;if(inspect)inspect(game.frame(),"hud-bottom-"+std::to_string(int(size.x)));
        std::optional<Rect> last;for(const auto& s:game.frame().sprites)if(s.image=="Btn_BuildMach.png"&&s.clip){const auto r=s.destination;if(r.y>=s.clip->y-.01f&&r.y+r.height<=s.clip->y+s.clip->height+.01f&&(!last||r.y>last->y))last=r;}
        require(last.has_value(),"No fully visible bottom HUD row");world->setValidationCells(10000);game.tick();tap(game,{292+tr.x,last->y+23.5f});
        const auto* expected=builds.find(full(owned.back().name,0));require(expected!=nullptr,"Missing last production definition");
        if(expected->placementType)tap(game,{120,260});
        require(world->productionRows().size()==count+1,"Bottom HUD item did not produce");require(world->productionRows().back().name==expected->name,"Bottom HUD produced wrong unit");
        require(host.writes==saves,"HUD production wrote save");++hudConditions;
    }
    std::ostringstream result;result<<"Level10/HUD: rows="<<cases.size()<<" purchases="<<bought<<" rejected="<<rejected<<" saveAndCodeReloads="<<reloads<<" inspectedFrames="<<frames<<" hudLayouts="<<hudConditions<<" statFrames="<<statFrames;
    return result.str();
}
}
