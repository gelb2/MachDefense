#include "SessionTestSupport.hpp"
#include <algorithm>
#include "Core/Simulation/AircraftVisual.hpp"
#include "Core/Simulation/AircraftModels.hpp"
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
   auto a=actual.pose(name,scale,angle,angle+.1f,angle+.2f),b=legacy.pose(name,scale,angle,angle+.1f,angle+.2f);
   const bool scoutUpgrade=std::string_view(base)=="AircraftLight"&&level>1;
   auto covers=std::count_if(a.begin(),a.end(),[](const auto& p){return p.definition->name.starts_with("ALightCover_");});assert(covers==(scoutUpgrade?1:0));
   for(const auto& p:a)if(scoutUpgrade&&p.definition->type==4)assert(p.definition->image==p.definition->name+".png");
   for(const auto& p:a)if(p.definition->name.starts_with("ALightCover_")){
    assert(p.definition->name=="ALightCover"+std::string(suffix)&&p.definition->tileCount==7&&p.definition->splitX==7);
    assert(p.definition->image==p.definition->name+".png");
    assert(std::abs(p.localCenter.x-scale*(std::cos(angle)-std::sin(angle)))<.0001f&&std::abs(p.localCenter.y-scale*(std::sin(angle)+std::cos(angle)))<.0001f&&p.angle==angle);
   }
   if(scoutUpgrade&&angle==0){
    Frame f;actual.appendSprites(f,a,{},[&](std::string_view n){return r.imageSize(n);});
    const Sprite *body=nullptr,*cover=nullptr;
    for(const auto& sprite:f.sprites)if(sprite.color.r>0){if(sprite.image=="ALightBody.png")body=&sprite;if(sprite.image.starts_with("ALightCover_"))cover=&sprite;}
    assert(body&&cover);assert(std::abs(body->destination.height-47*scale)<.0001f);
    assert(std::abs(cover->destination.x-body->destination.x-25*scale)<.0001f&&std::abs(cover->destination.y-body->destination.y-14*scale)<.0001f);
    for(float bank:{-.41887902f,-.27925268f,-.13962634f,0.f,.13962634f,.27925268f,.41887902f}){
     const auto mesh=aircraftBankMesh(*cover,{},0,bank,r.imageSize(cover->image));
     assert(std::abs(mesh.vertices.front().position.x-cover->destination.x)<.0001f&&std::abs(mesh.vertices.front().position.y-cover->destination.y)<.0001f);
     assert(std::abs(mesh.vertices.front().uv.x-float(aircraftBankTile(bank))/7.f)<.0001f);
    }
   }
   std::erase_if(a,[](const auto& p){return p.definition->name.starts_with("ALightCover_");});assert(a.size()==b.size());
   for(std::size_t i=0;i<a.size();++i){const auto& x=a[i];const auto& y=b[i];const auto& xd=*x.definition;const auto& yd=*y.definition;
    assert(xd.name==yd.name&&(xd.image==yd.image||(scoutUpgrade&&xd.type==4&&xd.image==xd.name+".png"))&&xd.type==yd.type&&xd.tileCount==yd.tileCount&&xd.splitX==yd.splitX&&xd.splitY==yd.splitY&&xd.tileNo==yd.tileNo);
    assert(x.localCenter.x==y.localCenter.x&&x.localCenter.y==y.localCenter.y&&x.angle==y.angle&&x.deviceScale==y.deviceScale&&x.visualScale==y.visualScale&&x.zOrder==y.zOrder&&x.reverse==y.reverse&&x.hasParent==y.hasParent);
    assert(xd.sockets.size()==yd.sockets.size());
    for(unsigned j=1;j<xd.sockets.size();++j){auto xm=x.muzzle(j),ym=y.muzzle(j);assert(xm&&ym&&xm->x==ym->x&&xm->y==ym->y);}
   }++cases;
  }
 }
 // Fixed AH-10 pods use the original muzzle coordinates despite moving their
 // image pivot to the complete body canvas. Check every level/roll/heading.
 unsigned fixedCases=0;
 for(unsigned level=1;level<=10;++level)for(float scale:{.5f,1.f,1.375f})for(float angle:{0.f,.7f,1.57f}){
  char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);
  auto tree=*actual.machine(std::string("AircraftHeavy")+suffix);
  for(auto& child:tree.front().children)if(child.name.starts_with("AHeavyRocket")||child.name.starts_with("AHeavyAirStrike"))child.angle=angle;
  auto poses=actual.pose(tree,scale,angle,angle+.8f,angle+.2f);
  const float k=scale*aircraftAssemblyScale("AHeavyBody");
  for(const auto& p:poses){
   bool rocket=p.definition->name.starts_with("AHeavyRocket"),strike=p.definition->name.starts_with("AHeavyAirStrike");
   if(!rocket&&!strike)continue;
   assert(std::abs(p.localCenter.x)<.0001f&&std::abs(p.localCenter.y)<.0001f&&p.angle==angle);
   assert(p.definition->tileCount==7&&p.definition->splitX==7);
   auto muzzle=p.muzzle();assert(muzzle);const Vec2 old=rocket?Vec2{17,34}:Vec2{36,-18};
   const Vec2 expected{(old.x*std::cos(angle)-old.y*std::sin(angle))*k,(old.x*std::sin(angle)+old.y*std::cos(angle))*k};
   assert(std::abs(muzzle->x-expected.x)<.0001f&&std::abs(muzzle->y-expected.y)<.0001f);
   for(float bank:{-.41887902f,-.27925268f,-.13962634f,0.f,.13962634f,.27925268f,.41887902f}){
    auto a=aircraftBankPoint(*muzzle,angle,bank),b=aircraftBankPoint(expected,angle,bank);
    assert(std::abs(a.x-b.x)<.0001f&&std::abs(a.y-b.y)<.0001f);++fixedCases;
   }
  }
 }
 std::cout<<fixedCases<<" AH-10 fixed pod muzzle cases retained across levels, scales, headings and bank poses\n";
 std::cout<<"40 persisted level assemblies; "<<cases<<" phone/pad pose cases preserve existing geometry, sockets and muzzles; only explicit Scout upgrade art differs\n";
}
