#include "Core/Data/DataTable.hpp"
#include "Core/Simulation/Combat.hpp"
#include "Core/Simulation/WeaponController.hpp"
#include "Core/Simulation/SpecialAttack.hpp"
#include <iostream>
#include <stdexcept>
int main(int argc,char**argv){
 auto table=md::DataTable::load(argv[1]);unsigned checked=0;
 std::cout<<"weapon,phone,mode,shots,max_per_tick,nominal_damage\n";
 for(bool phone:{false,true})for(size_t row=0;row<table.rowCount();++row){
  auto w=md::WeaponDefinition::fromTable(table,row,phone);md::DarwinRandom rng(123);
  unsigned shots=0,maxTick=0;std::string mode;
  if(w.name.starts_with("CF_")){
   auto bullets=md::crossfireProjectiles(w,{0,0},{10,10},0,0,rng);shots=bullets.size();maxTick=shots;mode="one-use";
   if(shots!=w.shotCount)throw std::runtime_error("Crossfire count");
  }else if(w.name.starts_with("AS_")){
   auto air=md::AirStrike::start(w,{0,0},0,0,phone?.5f:1.f,rng);mode="one-use";
   for(int tick=1;tick<=3600;++tick)if(air.tick(w,double(tick)*.0166,rng)){++shots;maxTick=1;}
   if(shots!=w.shotCount)throw std::runtime_error("Airstrike count");
  }else{
   md::WeaponController c;md::WeaponFrame f;f.target=md::WeaponTarget{{10,10},1000000};mode="3600-ticks";
   for(int tick=1;tick<=3600;++tick){f.time=double(tick)*.0166;unsigned n=0;auto r=c.tick(w,f,rng,[&](const auto&){++n;});if(r.shots!=n)throw std::runtime_error("Callback count");shots+=n;maxTick=std::max(maxTick,n);}
   if(!shots||maxTick>1)throw std::runtime_error("Normal fire scheduling");
  }
  std::cout<<w.name<<','<<phone<<','<<mode<<','<<shots<<','<<maxTick<<','<<long(shots)*w.damage<<'\n';++checked;
 }
 std::cerr<<"PASS "<<checked<<" weapon/geometry cases; no rendering or audio backend\n";
}
