#include "SessionTestSupport.hpp"
// Automated combat with explicit cheat starting conditions. Enemy bases must
// die from ordinary weapons/AI; this is not a normal-difficulty balance test.
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];Host h;auto save=SaveData::empty(1);save.createProfile("COMBAT",1);
    std::vector<OwnedBuild> loadout;for(auto name:{"MachineGun","HMG","Missile","HeavyMissile","ShotGun","PlasmaGun","IonRifle"})loadout.push_back({name,9,true});save.setBuilds(0,loadout);
    for(unsigned stage=0;stage<80;++stage){
        World world(r,h,save,0,BattleGeometry::phone(),1);world.start(stage);
        for(const auto& a:world.actors())if(a->base&&a->player)world.setValidationHealth(a->id,1000000);
        for(unsigned frame=0;frame<36000&&world.state()==WorldState::running;++frame){
            if(frame%60==0){unsigned live=0;for(const auto& a:world.actors())if(a->player&&!a->base&&!a->removed&&a->hp>0)++live;
                if(live<28)for(const auto& b:loadout){world.setValidationCells(100000);world.enqueueBuild(b.name);}}
            world.tick();world.prepareDraw();
        }
        std::cout<<"mission="<<stage+1<<" state="<<int(world.state())<<" time="<<world.time()-world.startTime()<<" produced="<<world.statistics().produced<<" kills="<<world.statistics().enemyDestroyed<<std::endl;
        assert(world.state()==WorldState::cleared);assert(world.statistics().produced>0);assert(h.writes==0);
    }
    std::cout<<"80 combat clears: original production, AI and weapon damage; level-10 loadout, cell resupply, boosted player-base HP\n";
}
