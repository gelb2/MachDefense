#include "Core/Simulation/Combat.hpp"
#include <iostream>
#include <stdexcept>
int main(int argc,char**argv){
 if(argc!=3)return 2;
 auto proposed=md::DataTable::load(argv[1]),expected=md::DataTable::load(argv[2]);
 auto pad=md::WeaponCatalog::load(proposed,false),phone=md::WeaponCatalog::load(proposed,true);
 unsigned checks=0;
 for(std::size_t i=0;i<expected.rowCount();++i){
  auto name=expected.value(i,"weapon");auto w=pad.find(name),p=phone.find(name);
  if(!w||!p)throw std::runtime_error("Missing weapon "+std::string(name));
  auto check=[&](int actual,const char* column){auto wanted=expected.integer(i,column);if(!wanted||actual!=*wanted)throw std::runtime_error(std::string(name)+" "+column+" actual="+std::to_string(actual));++checks;};
  check(w->damage,"damage");check(w->shotRange,"range");check(p->shotRange,"phoneRange");check(w->attackPoint(),"hudAttackPoint");check(w->shotCount,"burst");check(w->damageRange,"splashRadius");
 }
 std::cout<<"{\"status\":\"passed\",\"weaponRows\":"<<expected.rowCount()<<",\"checks\":"<<checks<<"}\n";
}
