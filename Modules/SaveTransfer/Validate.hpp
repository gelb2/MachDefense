#pragma once
#include "Codec.hpp"
#include "Core/Contracts/Game.hpp"
#include "Core/Simulation/Economy.hpp"
#include "Core/Session/Settlement.hpp"
#include <set>
namespace md::transfer {
inline void validate(const SaveData& s,ResourceAccess& r){
 auto table=[&](const char* name){auto b=r.read(std::string("Tables/")+name+".tsv");return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});};
 auto builds=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));Settlement settlement(builds,table("MachRegen"));BaseUpgradeCatalog bases(table("BaseUpgrade"));AttackCatalog attacks(table("SpAttackList"));
 for(size_t slot=0;slot<5;++slot)if(s.active(slot)){
  if(s.stage(slot)>79||s.name(slot).empty()||s.name(slot).size()>31)throw std::invalid_argument("Unsupported profile.");
  for(unsigned i=0;i<6;++i)if(s.baseUpgrade(slot,i)>bases.maximumLevel(BaseUpgrade(i)))throw std::invalid_argument("Unsupported base upgrade.");
  std::set<std::string> names;for(auto& b:s.builds(slot)){if(b.level>100||!settlement.definition(b)||!names.insert(b.name).second)throw std::invalid_argument("Unsupported unit upgrade.");}
  names.clear();for(auto& a:s.attacks(slot)){if(a.level>=attacks.maximumLevel(a.name)||a.count>2147483647u||!names.insert(a.name).second)throw std::invalid_argument("Unsupported special attack.");(void)attacks.definition(a);}
  const auto bytes=s.plaintext();auto offset=SaveData::slotStart+slot*SaveData::slotStride+0x30;
  std::string country;for(size_t i=0;i<3&&bytes[offset+i];++i)country+=char(bytes[offset+i]);
  if(!country.empty()&&r.imageSize(country+".png").x<=0)throw std::invalid_argument("Unsupported profile image.");
 }
}
}
