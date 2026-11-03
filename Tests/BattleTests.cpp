#include "../Sources/Core/Battle.hpp"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

std::vector<std::uint8_t> read(const std::filesystem::path& p) {
    std::ifstream f(p,std::ios::binary); if(!f) throw std::runtime_error("Missing test resource: "+p.string());
    return {std::istreambuf_iterator<char>(f),{}};
}
int main(int argc,char** argv) {
    assert(argc==2); std::filesystem::path root=argv[1];
    auto partsBytes=read(root/"PartsInfo.mcb"),machBytes=read(root/"MachInfo.mcb"),mapBytes=read(root/"MapInfo.mcb");
    auto parts=md::Chunk::parse(partsBytes),machines=md::Chunk::parse(machBytes),maps=md::Chunk::parse(mapBytes);
    assert(parts.children.size()==463 && machines.children.size()==126 && maps.children.size()==80);
    assert(parts.consumedSize==partsBytes.size() && parts.declaredSize!=parts.consumedSize);
    auto models=md::BattleModels::parse(parts,machines);
    assert(models.partCount()==463 && models.machineCount()==126);
    auto resourceRoot=root.parent_path();
    models.configureTiles(md::DataTable::load(resourceRoot/"Tables/PartsList.tsv"),md::DataTable::load(resourceRoot/"Tables/TileList.tsv"));
    auto imageSize=[&](std::string_view name) {
        if(!std::filesystem::exists(resourceRoot/"Images"/name)) return md::Vec2{};
        auto bytes=read(resourceRoot/"Images"/name);
        assert(bytes.size()>=24);
        auto be=[&](unsigned offset) { return std::uint32_t(bytes[offset])<<24 | std::uint32_t(bytes[offset+1])<<16 | std::uint32_t(bytes[offset+2])<<8 | bytes[offset+3]; };
        return md::Vec2{float(be(16)),float(be(20))};
    };
    std::size_t rendered=0;
    for(const auto& node:machines.children) {
        if(node.type!=3) continue;
        md::Frame frame;
        models.appendSprites(frame,node.text(0,24),{160,240},.5f,0,0,0,imageSize);
        if(node.text(0,24)=="Dummy") assert(frame.sprites.empty()); else assert(!frame.sprites.empty());
        for(const auto& sprite:frame.sprites) {
            auto size=imageSize(sprite.image);auto src=*sprite.sourcePx;
            assert(src.x>=0 && src.y>=0 && src.x+src.width<=size.x && src.y+src.height<=size.y);
        }
        ++rendered;
    }
    assert(rendered==126);
    auto dummy=models.part("DummyParts");assert(dummy && dummy->sockets.size()==4);
    assert(dummy->sockets[1].x==7 && dummy->sockets[1].y==9.5f);
    assert(dummy->socket(1,.5f).x==3.5f && dummy->socket(1,.5f).y==4.5f);
    auto player=models.machine("Player");assert(player && player->size()==1);
    assert((*player)[0].name=="DummyParts" && (*player)[0].children.size()==2);
    assert((*player)[0].children[0].parentSocket==1 && (*player)[0].children[1].parentSocket==2);
    // Dynamic source-built player parts preserve reverse sockets and per-part pose.
    md::AssemblyPart dynamicPart;dynamicPart.name="DummyParts";dynamicPart.reverse=2;
    dynamicPart.angle=0;dynamicPart.addPosition={3,4};dynamicPart.visualScale=1.2f;
    auto dynamicPose=models.pose(std::vector<md::AssemblyPart>{dynamicPart},.5f,0,0,0);
    assert(dynamicPose.size()==1&&dynamicPose[0].reverse==2&&dynamicPose[0].visualScale==1.2f);
    const auto anchor=dummy->socket(0,.5f);
    assert(dynamicPose[0].localCenter.x==3-anchor.x&&dynamicPose[0].localCenter.y==4+anchor.y);
    auto muzzle=dynamicPose[0].muzzle(1);assert(muzzle);
    assert(muzzle->x==dynamicPose[0].localCenter.x+3.5f-anchor.x);
    assert(muzzle->y==dynamicPose[0].localCenter.y+anchor.y-4.5f);
    bool truncated=false; partsBytes.pop_back();
    try{md::Chunk::parse(partsBytes);}catch(const std::runtime_error&){truncated=true;}assert(truncated);
    md::BattleClock clock;clock.start(100,3);assert(clock.cells()==40);
    clock.tick(true,false,0);assert(clock.cells()==43 && clock.time()==double(.0166f));
    double t=clock.time();clock.tick(true,true,0);clock.tick(false,false,0);assert(clock.time()==t);
    clock.setCells(99);for(int i=0;i<10;++i) clock.tick(true,false,0);assert(clock.cells()==102);
    clock.start(100,3);clock.tick(true,false,1);assert(clock.cells()==40 && clock.time()>0);
    assert(md::BattleClock::enemyBaseHP(0)==40000 && md::BattleClock::enemyBaseHP(79)==830000);
    assert(md::BattleClock::initialDifficultyScale(0)==.3f && md::BattleClock::initialDifficultyScale(5)==1.f);
    auto job=md::ProductionJob::begin("MachineGun_01",0,3,1);
    assert(!job.complete(3));assert(job.complete(3.000001));
    md::DarwinRandom random(1);md::EnemySpawner spawner({1,0,"E_Tank_01",2,7},0,random);
    assert(!spawner.tick(1,0,md::BattleGeometry::phone(),random));
    assert(!spawner.tick(2,1,md::BattleGeometry::phone(),random));
    auto event=spawner.tick(2,0,md::BattleGeometry::phone(),random);assert(event);
    assert(event->worldPosition.x>=-110 && event->worldPosition.x<=110 && event->worldPosition.y==528);
    assert(spawner.nextSpawnTime()>=5.5 && spawner.nextSpawnTime()<=9);
    assert(spawner.tick(10,0,md::BattleGeometry::pad(),random)->worldPosition.y==1056);assert(spawner.finished());
    assert(!spawner.tick(100,0,md::BattleGeometry::pad(),random));
    md::EnemySpawner dynamic({-1,0,"E_Tank_01",10,7},0,random);assert(dynamic.targetCount()>=5&&dynamic.targetCount()<=10);
    auto machList=md::DataTable::load(resourceRoot/"Tables/MachList.tsv");
    auto tankRow=machList.findRow("E_Tank_01");assert(tankRow);
    auto tank=md::MachineDefinition::fromTable(machList,*tankRow,md::BattleGeometry::phone());
    assert(tank.hp==*machList.integer(*tankRow,"HP")*2 && tank.type==1);
    auto padTank=md::MachineDefinition::fromTable(machList,*tankRow,md::BattleGeometry::pad());
    assert(padTank.speed==tank.speed*2);
    md::VehicleMotion vehicle;vehicle.setDestination({1000,0});md::DarwinRandom movementRandom(1);
    vehicle.tick(movementRandom);assert(vehicle.position.x==.006f && vehicle.position.y==0);
    vehicle.setDestination({vehicle.position.x+4,vehicle.position.y+4});vehicle.tick(movementRandom);
    assert(vehicle.state==0 && vehicle.speed==0);
    std::vector<md::TargetCandidate> candidates={{{3,4},10},{{0,0},0},{{-3,-4},10}};
    assert(!md::nearestLivingTarget({},5,candidates));
    assert(md::nearestLivingTarget({},6,candidates)==0);
    assert(md::hpAfterDamage(20,30)==0 && md::hpAfterDamage(20,5)==15);
    for(int stage=0;stage<80;++stage) {
        char filename[32];std::snprintf(filename,sizeof(filename),"ESpawn_%02d.tsv",stage+1);
        auto plan=md::StageSpawnPlan::parse(maps.children[stage],md::DataTable::load(resourceRoot/"Tables"/filename),md::BattleGeometry::phone());
        assert(!plan.enemyNames.empty() && !plan.difficulty[0].empty());
        auto timed=plan.timed.size();md::DarwinRandom waves(1);
        plan.appendGeneratedWaves(stage,waves,[&](std::string_view name){auto row=machList.findRow(name);return row?md::MachineDefinition::fromTable(machList,*row,md::BattleGeometry::phone()).hp:0;});
        assert(plan.timed.size()>timed && plan.timed.size()<=timed+28);
        for(auto& rule:plan.timed) assert(rule.count>0 && std::isfinite(rule.startTime) && rule.delay>=0);
        md::EnemyDirector director(plan.difficulty);assert(!director.tick(0,waves));assert(director.tick(.0166,waves));
    }
    md::AircraftMotion plane;plane.setDestination({1000,0});plane.tick();assert(plane.position.x==.006f);
    auto approachSpeed=plane.speed;plane.setDestination(plane.position);plane.tick();assert(plane.state==0 && plane.speed==approachSpeed);
    md::DarwinRandom steeringRandom(1);auto steering=md::EnemySteering::begin(0,steeringRandom);
    assert(steering.stopDelay>=.5f && steering.stopDelay<=1.5f);
    assert(!steering.vehicle(0,{0,0},100,487,.5f,false,false,steeringRandom));
    auto destination=steering.vehicle(2,{100,-480},100,487,.5f,false,false,steeringRandom);
    assert(destination && destination->x==55 && destination->y==-407 && steering.stopDelay==100002.f);
    auto flight=steering.aircraft(2,0,steeringRandom);assert(flight && flight->x>=-600&&flight->x<=600&&flight->y>=-600&&flight->y<=600);
    assert(md::originalStepSpeedPoint(0)==0&&md::originalStepSpeedPoint(.5f)==30);
    assert(md::originalStepSpeedPoint(1)==8&&md::originalVehicleSpeedPoint(.6f)==36);
    md::TrainMotion train;train.setDestination({600,800});train.tick();
    assert(std::abs(train.position.x-.0036f)<1e-8f&&std::abs(train.position.y-.0048f)<1e-8f);
    train.setDestination(train.position);train.tick();assert(train.state==0&&train.speed==0);
    auto segment=md::TerrainSegment::between({0,0},{100,0});
    assert(segment.normal.x==0&&segment.normal.y==-1&&segment.length==100);
    assert(segment.penetration({50,10})==6&&segment.penetration({50,-10})==6);
    assert(segment.penetration({-1,0})==0&&segment.penetration({117,0})==0);
    assert(segment.penetration({116,0})==16&&segment.penetration({50,16})==0);
    auto previous=md::TerrainSegment::between({0,100},{0,0});segment.setPrevious(&previous);
    assert(segment.penetration({-1,-10})==6&&segment.penetration({1,10})==0);
    std::cout<<"Battle foundations: MCB tree/assembly, time, cells, production and spawn checks passed\n";
}
