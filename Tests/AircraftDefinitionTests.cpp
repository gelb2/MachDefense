#include "SessionTestSupport.hpp"
#include <algorithm>
// A data-only load must contain every campaign level without runtime cloning.
// Compare against the previous assembly path to protect geometry and image use.
int main(int argc,char**argv){
 assert(argc==2);Resources r;r.root=argv[1];
 auto parts=Chunk::parse(r.read("RawData/AircraftParts.mcb"));
 auto machines=Chunk::parse(r.read("RawData/AircraftMachines.mcb"));
 auto persisted=BattleModels::parse(parts,machines);
 auto isLevel=[](const Chunk& c){auto n=c.text(0,24);return n.size()>3&&n[n.size()-3]=='_'&&n[n.size()-2]>='0'&&n[n.size()-2]<='9'&&n.back()>='0'&&n.back()<='9';};
 std::erase_if(parts.children,isLevel);std::erase_if(machines.children,isLevel);
 auto legacy=BattleModels::parse(parts,machines);
 legacy.configureTiles(DataTable::load(r.root/"Tables/PartsList.tsv"),DataTable::load(r.root/"Tables/TileList.tsv"));
 auto actual=BattleModels::load(r);unsigned cases=0;
 for(const char* base:{"AircraftLight","AircraftHeavy","AircraftDuct","AircraftTransport"})for(unsigned level=1;level<=10;++level){
  char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);auto name=std::string(base)+suffix;
  assert(persisted.machine(name));legacy.appendMachineVariant(name,base,suffix);
  for(float scale:{.5f,1.f})for(float angle:{0.f,.7f,1.57f}){
   auto a=actual.pose(name,scale,angle,angle+.1f,angle+.2f),b=legacy.pose(name,scale,angle,angle+.1f,angle+.2f);assert(a.size()==b.size());
   for(std::size_t i=0;i<a.size();++i){const auto& x=a[i];const auto& y=b[i];const auto& xd=*x.definition;const auto& yd=*y.definition;
    assert(xd.name==yd.name&&xd.image==yd.image&&xd.type==yd.type&&xd.tileCount==yd.tileCount&&xd.splitX==yd.splitX&&xd.splitY==yd.splitY&&xd.tileNo==yd.tileNo);
    assert(x.localCenter.x==y.localCenter.x&&x.localCenter.y==y.localCenter.y&&x.angle==y.angle&&x.deviceScale==y.deviceScale&&x.visualScale==y.visualScale&&x.zOrder==y.zOrder&&x.reverse==y.reverse&&x.hasParent==y.hasParent);
    assert(xd.sockets.size()==yd.sockets.size());
    for(unsigned j=1;j<xd.sockets.size();++j){auto xm=x.muzzle(j),ym=y.muzzle(j);assert(xm&&ym&&xm->x==ym->x&&xm->y==ym->y);}
   }++cases;
  }
 }
 std::cout<<"40 persisted level assemblies; "<<cases<<" phone/pad pose cases match previous images, geometry, sockets and muzzles\n";
}
