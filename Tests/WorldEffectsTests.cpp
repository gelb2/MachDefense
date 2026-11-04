#include "Core/Simulation/World.hpp"
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
int main(int argc,char**argv){assert(argc==2);Resources resources;resources.root=argv[1];Host host;auto save=md::SaveData::empty(1);save.createProfile("PLAYER",1);
    md::World world(resources,host,save,0);world.start(0);
    world.setValidationCells(1000);assert(world.cells()==1000);assert(world.upgradeCells());
    bool rejected=false;try{world.setValidationCells(-1);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    md::ActorId playerBase=0,enemyBase=0;for(const auto& a:world.actors())if(a->base)(a->player?playerBase:enemyBase)=a->id;
    auto* friendly=world.actor(playerBase);auto* hostile=world.actor(enemyBase);
    assert(world.setValidationHealth(enemyBase,10000000));assert(hostile->hp==10000000);
    hostile->position={friendly->position.x,friendly->position.y+60};
    bool muzzle=false,impact=false,line=false,shell=false;
    for(int i=0;i<300&&(!muzzle||!impact||!line||!shell);++i){world.tick();md::Frame frame;world.appendSprites(frame);for(const auto& sprite:frame.sprites){shell|=sprite.image=="BulletShell.png";muzzle|=sprite.image=="Efx_Fire.png";line|=sprite.image=="Efx_BulletTrail.png";impact|=sprite.image=="Efx_Damage.png"||sprite.image=="Efx_DamageHMG.png";}}
    assert(muzzle&&impact&&line&&shell);
    const auto bot=world.validationSpawn("Bot_A01",true,{0,0});assert(world.actor(bot)&&world.actor(bot)->player);
    assert(world.setValidationHealth(bot,0)&&world.actor(bot)->hp==0);assert(!world.setValidationHealth(999999,10));
    auto attacks=save.attacks(0);attacks.push_back({"AirStrike-EMP",0,1,true});save.setAttacks(0,attacks);
    assert(world.activateSpecial("AirStrike-EMP",{0,0}));bool pulse=false,boss=false;
    for(int i=0;i<600&&(!pulse||!boss);++i){world.tick();md::Frame frame;world.appendSprites(frame);for(const auto& sprite:frame.sprites){pulse|=sprite.image=="Efx_EMP.png";boss|=sprite.image=="Efx_ExplodeBoss.png";}}
    assert(!pulse&&boss&&host.writes==0);assert(!std::filesystem::exists(resources.root/"Images/Efx_EMP.png"));
    md::World trailWorld(resources,host,save,0);trailWorld.start(0);trailWorld.setValidationCells(1000);
    assert(trailWorld.enqueueBuild(save.builds(0).front().name));md::WorldActor* unit=nullptr;
    for(int i=0;i<900&&!unit;++i){trailWorld.tick();for(const auto& a:trailWorld.actors())if(a->player&&!a->base&&a->type==0){unit=a.get();break;}}
    assert(unit);unit->position={0,-300};attacks=save.attacks(0);attacks.push_back({"Crossfire-Missile",0,2,true});save.setAttacks(0,attacks);
    assert(trailWorld.activateSpecial("Crossfire-Missile",{0,-190}));bool mesh=false;
    for(int i=0;i<120;++i){trailWorld.tick();trailWorld.prepareDraw();md::Frame f;trailWorld.appendSprites(f);for(const auto& m:f.meshes){mesh=true;assert(m.beforeSpriteIndex<=f.sprites.size());for(const auto& v:m.vertices)assert(std::isfinite(v.position.x)&&std::isfinite(v.position.y));}}
    assert(mesh);assert(trailWorld.setValidationHealth(unit->id,0));assert(!unit->removed&&unit->crashTime>trailWorld.time()+2.9);
    const auto wreckUntil=unit->crashTime;bool wreckSmoke=false,wreckExplosion=false;
    for(int i=0;i<400&&!unit->removed;++i){trailWorld.tick();md::Frame f;trailWorld.appendSprites(f);for(const auto& sprite:f.sprites){wreckSmoke|=sprite.image=="Efx_Smoke.png";wreckExplosion|=sprite.image=="Efx_ExplodeNPC.png";}if(trailWorld.time()<=wreckUntil)assert(!unit->removed);}
    assert(unit->removed&&wreckSmoke&&wreckExplosion&&host.writes==0);
    std::cout<<"World validation setters, muzzle flash, weapon impact and shells, original EMP, missile meshes and wreck lifetime passed\n";
}
