#include "Core/Simulation/World.hpp"
#include "NuclearDamageSweep.hpp"
#include "BaseUpgradeEffectSweep.hpp"
#include "DamageAudioSweep.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
struct Resources:md::ResourceAccess{
    std::filesystem::path root;std::unordered_map<std::string,md::Vec2> sizes;
    md::Bytes read(std::string_view path)override{std::ifstream f(root/path,std::ios::binary);if(!f)throw std::runtime_error(std::string(path));return {std::istreambuf_iterator<char>(f),{}};}
    md::Vec2 imageSize(std::string_view name)override{auto it=sizes.find(std::string(name));if(it!=sizes.end())return it->second;if(!std::filesystem::exists(root/"Images"/name))return {};auto b=read("Images/"+std::string(name));if(b.size()>24&&b[0]==137){auto be=[&](int i){return unsigned(b[i])<<24|unsigned(b[i+1])<<16|unsigned(b[i+2])<<8|b[i+3];};return sizes[std::string(name)]={float(be(16)),float(be(20))};}return {};}
};
struct Host:md::HostServices{int writes=0,sounds=0;std::optional<md::Bytes>readSave()override{return{};}bool writeSave(std::span<const std::uint8_t>)override{++writes;return true;}void playSound(std::string_view,float,float)override{++sounds;}void playMusic(std::string_view,float)override{}void stopMusic()override{}void requestName(std::string,std::size_t,std::function<void(std::optional<std::string>)>)override{}};
int main(int argc,char**argv){assert(argc==2);Resources resources;resources.root=argv[1];Host host;
    std::cout<<md::validation::runNuclearDamageSweep(resources,host);assert(host.writes==0);
    std::cout<<md::validation::runBaseUpgradeEffectSweep(resources,host);assert(host.writes==0);
    std::cout<<md::validation::runDamageAudioSweep(resources,host);assert(host.writes==0);
    for(auto geometry:{md::BattleGeometry::phone(),md::BattleGeometry::pad()}){
        auto save=md::SaveData::empty(1);save.createProfile("PLAYER",1);
        md::World world(resources,host,save,0,geometry,123);world.start(0);
        const float h=geometry.deviceScale==.5f?480.f:1024.f,margin=geometry.deviceScale==.5f?62.f:178.f;
        assert(world.cameraTargetY()==h*.5f-margin-geometry.halfLength());
        world.setUIMargin(margin+20);world.tick();assert(world.cameraY()==h*.5f-margin-20-geometry.halfLength());
        world.setUIMargin(margin);world.tick();assert(world.cameraY()==h*.5f-margin-geometry.halfLength());
        auto attacks=save.attacks(0);const auto initial=attacks[0].count;
        assert(world.activateSpecial(attacks[0].name,{0,0}));assert(save.attacks(0)[0].count==initial-1);
        md::Frame frame;world.appendSprites(frame);bool plane=false,marker=false;
        for(const auto& s:frame.sprites){plane|=s.image=="Airplane.png";marker|=s.image=="Target.png";}assert(plane&&marker);
        bool drop=false;for(int i=0;i<120;++i){world.tick();for(const auto& b:world.projectiles())drop|=b.motion.drop;}assert(drop);
        world.start(0);attacks=save.attacks(0);attacks.push_back({"Crossfire-Missile",0,3,true});save.setAttacks(0,attacks);
        assert(!world.activateSpecial("Crossfire-Missile",{0,0}));assert(save.attacks(0).back().count==3);
        const auto builds=save.builds(0);assert(world.enqueueBuild(builds[0].name));
        md::WorldActor* unit=nullptr;
        for(int i=0;i<900&&!unit;++i){world.tick();for(const auto& a:world.actors())if(a->player&&!a->base&&a->type==0){unit=a.get();break;}}
        assert(unit);unit->position={0,-300};
        assert(!world.activateSpecial("Crossfire-Missile",{0,-301}));assert(save.attacks(0).back().count==3);
        assert(!world.activateSpecial("Crossfire-Missile",{0,-200}));assert(save.attacks(0).back().count==3);
        assert(world.activateSpecial("Crossfire-Missile",{0,-190}));assert(save.attacks(0).back().count==2);
        bool lock=false;for(const auto& p:world.projectiles())lock|=p.motion.lockOn;assert(lock);
        world.setPaused(true);assert(!world.activateSpecial(attacks[0].name,{0,0}));assert(host.writes==0);
        world.start(0);assert(world.cellLevel()==0&&world.cellCapacity()==500&&world.cellUpgradeCost()==300);
        assert(!world.upgradeCells());const auto permanent=save.baseUpgrade(0,5);const auto credits=save.credits(0);
        for(unsigned level=0;level<5;++level){
            int guard=50000;
            while(world.cells()<float(world.cellUpgradeCost())&&--guard){for(const auto& a:world.actors())if(!a->player)a->hp=0;world.tick();}
            assert(guard>0);const float beforeCells=world.cells();const int cost=world.cellUpgradeCost();
            assert(world.upgradeCells()&&world.cellLevel()==level+1&&world.cells()==beforeCells-float(cost));
            assert(world.cellCapacity()==500.f*float(level+2));assert(save.baseUpgrade(0,5)==permanent&&save.credits(0)==credits);
        }
        assert(!world.upgradeCells());world.start(0);assert(world.cellLevel()==0&&world.cellCapacity()==500);
        md::ActorId base=0;for(const auto& a:world.actors())if(a->base&&!a->player){base=a->id;a->coin=7;}
        const auto scoreOffset=md::SaveData::slotStart+0x2054;
        const auto scoreBefore=save.readWord(scoreOffset);bool callback=false;
        world.setFinishHandler([&](md::WorldState state){assert(state==md::WorldState::cleared);callback=true;assert(world.statistics().enemyDestroyed==0);assert(save.readWord(scoreOffset)==scoreBefore);});
        world.damage(base,10000000);assert(callback&&world.statistics().enemyDestroyed>=1&&save.readWord(scoreOffset)>=scoreBefore+7);

    }
    std::cout<<"World special attack inventory, qualification, projectile, sprite and UI margin tests passed\n";
}
