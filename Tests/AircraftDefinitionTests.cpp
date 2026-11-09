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
 std::function<void(Chunk&)> stripCovers=[&](Chunk& c){std::erase_if(c.children,[](const Chunk& n){return n.type==4&&(n.text(0,24).starts_with("ALightCover_")||n.text(0,24).starts_with("AHeavyCover_")||n.text(0,24).starts_with("ADuctCover_")||n.text(0,24).starts_with("ATransportRoot"));});for(auto& n:c.children)stripCovers(n);};
 stripCovers(machines);
 auto legacy=BattleModels::parse(parts,machines);
 legacy.configureTiles(DataTable::load(r.root/"Tables/PartsList.tsv"),DataTable::load(r.root/"Tables/TileList.tsv"));
 auto actual=BattleModels::load(r);unsigned cases=0,v5Cases=0;
 for(const char* base:{"AircraftLight","AircraftHeavy","AircraftDuct","AircraftTransport"})for(unsigned level=1;level<=10;++level){
  char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);auto name=std::string(base)+suffix;
  assert(persisted.machine(name));legacy.appendMachineVariant(name,base,suffix);
  for(float scale:{.5f,1.f})for(float angle:{0.f,.7f,1.57f}){
   auto a=actual.pose(name,scale,angle,angle+.1f,angle+.2f),b=legacy.pose(name,scale,angle,angle+.1f,angle+.2f);
   const bool heavyUpgrade=std::string_view(base)=="AircraftHeavy";
   const bool belugaUpgrade=std::string_view(base)=="AircraftTransport";
   const bool cheyenneUpgrade=std::string_view(base)=="AircraftDuct";
   const bool scoutUpgrade=std::string_view(base)=="AircraftLight";
   if(helicopterV5Body(actual.machine(name)->front().name)){
    assert(belugaUpgrade||(scoutUpgrade&&level==10));
    const float k=scale*aircraftAssemblyScale(belugaUpgrade?"ATransportBody":"ALightBody");
    unsigned bodies=0,rotors=0;std::vector<std::string> weapons,oldWeapons;
    for(const auto& p:b)if(p.definition->type==4)oldWeapons.push_back(p.definition->name);
    for(const auto& p:a){
     const auto& d=*p.definition;const auto size=r.imageSize(d.image);
     assert(size.x>0&&size.y>0&&std::abs(p.deviceScale-k)<.000001f);
     if(helicopterV5Body(d.name)){++bodies;assert(d.type==0);}
     if(helicopterDrawnPart(d.name))assert(d.tileCount==7&&d.splitX==7&&d.splitY==1);
     if(d.type==7){++rotors;assert(d.tileCount==1&&d.splitX==1&&d.splitY==1&&p.angle==angle+.2f);}
     if(d.type==4){
      weapons.push_back(d.name);assert(p.angle==angle+.1f&&d.tileCount==1&&d.splitX==1&&d.splitY==1);
      assert(d.sockets.size()>=2);
      for(unsigned j=1;j<d.sockets.size();++j){
       const auto socket=d.sockets[j],pivot=d.sockets[0];const auto muzzle=p.muzzle(j);assert(muzzle);
       assert(socket.x>pivot.x&&std::abs(socket.x)<=size.x*.5f+.51f&&std::abs(socket.y)<=size.y*.5f+.51f);
       const Vec2 offset{(socket.x-pivot.x)*k,(p.reverse?-1.f:1.f)*(socket.y-pivot.y)*k};
       const Vec2 expected{p.localCenter.x+offset.x*std::cos(p.angle)-offset.y*std::sin(p.angle),p.localCenter.y+offset.x*std::sin(p.angle)+offset.y*std::cos(p.angle)};
       assert(std::abs(muzzle->x-expected.x)<.0001f&&std::abs(muzzle->y-expected.y)<.0001f);
      }
     }
    }
    std::sort(weapons.begin(),weapons.end());std::sort(oldWeapons.begin(),oldWeapons.end());
    assert(bodies==1&&rotors==1&&weapons==oldWeapons&&weapons.size()==(scoutUpgrade?2:4));
    Frame f;actual.appendSprites(f,a,{},[&](std::string_view n){return r.imageSize(n);});
    for(const auto& sprite:f.sprites){
     const auto partName=std::string_view(sprite.image).substr(0,sprite.image.size()-4);
     if(!helicopterDrawnPart(partName))continue;
     const auto size=r.imageSize(sprite.image);assert(sprite.sourcePx);
     const auto source=*sprite.sourcePx;assert(source.width==size.x/7&&source.height==size.y&&source.x==0);
     for(unsigned tile=0;tile<7;++tile){
      const float bank=tile==0?0.f:(tile<=3?-float(tile):float(tile-3))*.13962634f;
      const auto mesh=aircraftBankMesh(sprite,{},angle,bank,size),neutral=aircraftBankMesh(sprite,{},angle,0,size);
      for(unsigned vertex=0;vertex<4;++vertex){
       assert(mesh.vertices[vertex].position.x==neutral.vertices[vertex].position.x&&mesh.vertices[vertex].position.y==neutral.vertices[vertex].position.y);
       assert(std::abs(mesh.vertices[vertex].uv.x-neutral.vertices[vertex].uv.x-float(tile)/7)<.000001f);
       assert(mesh.vertices[vertex].uv.x>=0&&mesh.vertices[vertex].uv.x<=1);
      }
     }
    }
    ++v5Cases;continue;
   }
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
   if(cheyenneUpgrade){
    unsigned covers=0;
    const float k=scale*aircraftAssemblyScale("ADuctBody");
    for(const auto& p:a){
     if(p.definition->type==4)assert(p.definition->image=="ADuctMissile"+std::string(suffix)+".png");
     if(!p.definition->name.starts_with("ADuctCover_"))continue;
     ++covers;assert(p.definition->name=="ADuctCover"+std::string(suffix)&&p.definition->tileCount==7&&p.definition->splitX==7);
     assert(std::abs(p.localCenter.x+6*k*std::cos(angle))<.0001f&&std::abs(p.localCenter.y+6*k*std::sin(angle))<.0001f);
    }
    assert(covers==1);
    if(angle==0){
     Frame f;actual.appendSprites(f,a,{},[&](std::string_view n){return r.imageSize(n);});const Sprite *body=nullptr,*cover=nullptr;
     for(const auto& sprite:f.sprites)if(sprite.color.r>0){if(sprite.image=="ADuctBody.png")body=&sprite;if(sprite.image.starts_with("ADuctCover_"))cover=&sprite;}
     assert(body&&cover);assert(std::abs(cover->destination.x-body->destination.x-35*k)<.0001f&&std::abs(cover->destination.y-body->destination.y-37*k)<.0001f);
     for(float bank:{-.41887902f,-.27925268f,-.13962634f,0.f,.13962634f,.27925268f,.41887902f}){
      const auto mesh=aircraftBankMesh(*cover,{},0,bank,r.imageSize(cover->image));
      assert(std::abs(mesh.vertices.front().position.x-cover->destination.x)<.0001f&&std::abs(mesh.vertices.front().position.y-cover->destination.y)<.0001f);
      assert(std::abs(mesh.vertices.front().uv.x-float(aircraftBankTile(bank))/7.f)<.0001f);
     }
    }
   }
   if(belugaUpgrade){
    unsigned count=0;const float k=scale*aircraftAssemblyScale("ATransportBody");
    for(const auto& p:a)if(p.definition->name.starts_with("ATransportRoot")){
     const bool top=p.definition->name.starts_with("ATransportRoot0_");const float ly=top?23.f:-25.f;
     assert(p.definition->name==std::string(top?"ATransportRoot0":"ATransportRoot1")+suffix);
     assert(p.definition->tileCount==7&&p.definition->splitX==7);
     assert(std::abs(p.localCenter.x-k*(10*std::cos(angle)-ly*std::sin(angle)))<.0001f);
     assert(std::abs(p.localCenter.y-k*(10*std::sin(angle)+ly*std::cos(angle)))<.0001f);++count;
    }
    assert(count==2);
    if(angle==0){
     Frame f;actual.appendSprites(f,a,{},[&](std::string_view n){return r.imageSize(n);});const Sprite* body=nullptr;
     for(const auto& sprite:f.sprites)if(sprite.color.r>0&&sprite.image=="ATransportBody.png")body=&sprite;
     assert(body);
     for(const auto& sprite:f.sprites)if(sprite.color.r>0&&sprite.image.starts_with("ATransportRoot")){
      const float y0=sprite.image.starts_with("ATransportRoot0_")?28.f:76.f;
      assert(std::abs(sprite.destination.x-body->destination.x-64*k)<.0001f&&std::abs(sprite.destination.y-body->destination.y-y0*k)<.0001f);
      for(float bank:{-.41887902f,-.27925268f,-.13962634f,0.f,.13962634f,.27925268f,.41887902f}){
       const auto mesh=aircraftBankMesh(sprite,{},0,bank,r.imageSize(sprite.image));
       assert(std::abs(mesh.vertices.front().position.x-sprite.destination.x)<.0001f&&std::abs(mesh.vertices.front().position.y-sprite.destination.y)<.0001f);
       assert(std::abs(mesh.vertices.front().uv.x-float(aircraftBankTile(bank))/7.f)<.0001f);
      }
     }
    }
   }
   if(heavyUpgrade){
    unsigned count=0;const float k=scale*aircraftAssemblyScale("AHeavyBody");
    for(const auto& p:a){
     const auto n=p.definition->name;
     if(n.starts_with("AHeavyGun_")||n.starts_with("AHeavyCannon_")||n.starts_with("AHeavyMissile_"))assert(p.definition->image==n+".png");
     if(!n.starts_with("AHeavyCover_"))continue;
     ++count;assert(n=="AHeavyCover"+std::string(suffix)&&p.definition->tileCount==7&&p.definition->splitX==7);
     assert(std::abs(p.localCenter.x-2*k*(std::cos(angle)+std::sin(angle)))<.0001f);
     assert(std::abs(p.localCenter.y-2*k*(std::sin(angle)-std::cos(angle)))<.0001f&&p.angle==angle);
    }
    assert(count==1);
    if(angle==0){
     Frame f;actual.appendSprites(f,a,{},[&](std::string_view n){return r.imageSize(n);});const Sprite *body=nullptr,*cover=nullptr;
     for(const auto& sprite:f.sprites)if(sprite.color.r>0){if(sprite.image=="AHeavyBody.png"&&sprite.sourcePx->x==0)body=&sprite;if(sprite.image.starts_with("AHeavyCover_"))cover=&sprite;}
     assert(body&&cover);assert(std::abs(cover->destination.x-body->destination.x-28*k)<.0001f&&std::abs(cover->destination.y-body->destination.y-47*k)<.0001f);
     for(float bank:{-.41887902f,-.27925268f,-.13962634f,0.f,.13962634f,.27925268f,.41887902f}){
      const auto mesh=aircraftBankMesh(*cover,{},0,bank,r.imageSize(cover->image));
      assert(std::abs(mesh.vertices.front().position.x-cover->destination.x)<.0001f&&std::abs(mesh.vertices.front().position.y-cover->destination.y)<.0001f);
      assert(std::abs(mesh.vertices.front().uv.x-float(aircraftBankTile(bank))/7.f)<.0001f);
     }
    }
   }
   std::erase_if(a,[](const auto& p){return p.definition->name.starts_with("ALightCover_")||p.definition->name.starts_with("ADuctCover_")||p.definition->name.starts_with("ATransportRoot")||p.definition->name.starts_with("AHeavyCover_");});assert(a.size()==b.size());
   for(std::size_t i=0;i<a.size();++i){const auto& x=a[i];const auto& y=b[i];const auto& xd=*x.definition;const auto& yd=*y.definition;
    assert(xd.name==yd.name&&(xd.image==yd.image||(scoutUpgrade&&xd.type==4&&xd.image==xd.name+".png")||(cheyenneUpgrade&&xd.type==4&&xd.image=="ADuctMissile"+std::string(suffix)+".png")||(heavyUpgrade&&(xd.name.starts_with("AHeavyGun_")||xd.name.starts_with("AHeavyCannon_")||xd.name.starts_with("AHeavyMissile_"))&&xd.image==xd.name+".png"))&&xd.type==yd.type&&xd.tileCount==yd.tileCount&&xd.splitX==yd.splitX&&xd.splitY==yd.splitY&&xd.tileNo==yd.tileNo);
    assert(x.localCenter.x==y.localCenter.x&&x.localCenter.y==y.localCenter.y&&x.angle==y.angle&&x.deviceScale==y.deviceScale&&x.visualScale==y.visualScale&&x.zOrder==y.zOrder&&x.reverse==y.reverse&&x.hasParent==y.hasParent);
    assert(xd.sockets.size()==yd.sockets.size());
    for(unsigned j=1;j<xd.sockets.size();++j){auto xm=x.muzzle(j),ym=y.muzzle(j);assert(xm&&ym&&xm->x==ym->x&&xm->y==ym->y);}
   }++cases;
  }
 }
 // All three independent nodes consume distinct full-size strips of one atlas.
 for(unsigned level=1;level<=10;++level){
  char suffix[8];snprintf(suffix,sizeof suffix,"_%02u",level);
  auto poses=actual.pose(std::string("AircraftHeavy")+suffix,1.f,0,0,0);
  Frame frame;actual.appendSprites(frame,poses,{},[&](std::string_view n){return r.imageSize(n);});
  unsigned visible=0;
  for(const auto& sprite:frame.sprites)if(sprite.image=="AHeavyBody.png"&&sprite.color.r>0){
   assert(sprite.sourcePx->width==76&&sprite.sourcePx->height==112);
   assert(sprite.sourcePx->x==0||sprite.sourcePx->x==532||sprite.sourcePx->x==1064);
   for(float bank:{-.41887902f,-.27925268f,-.13962634f,0.f,.13962634f,.27925268f,.41887902f}){
    auto mesh=aircraftBankMesh(sprite,{},0,bank,r.imageSize(sprite.image));
    assert(std::abs(mesh.vertices.front().uv.x-(sprite.sourcePx->x+aircraftBankTile(bank)*76)/1596.f)<.0001f);
   }
   ++visible;
  }
  assert(visible==3);
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
   assert(p.definition->tileCount==7&&p.definition->splitX==21&&p.definition->splitY==1);
   assert(p.definition->image=="AHeavyBody.png"&&p.definition->tileNo==(rocket?7:14));
   auto imageSize=r.imageSize(p.definition->image);assert(imageSize.x/21==76&&imageSize.y==112);
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
 std::cout<<"40 persisted level assemblies; "<<cases<<" unchanged phone/pad pose cases and "<<v5Cases<<" V5 authored-part cases checked\n";
}
