#pragma once
#include "Core/Simulation/Battle.hpp"
#include "Core/Simulation/AircraftModels.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include <algorithm>
#include "AircraftComparisonScene.hpp"
namespace md::validation {
// Cheat-only measured sheet: production parts and sprite assembly, uniform scale.
inline Frame aircraftVisualFrame(ResourceAccess& r,unsigned mode){
 if(mode>=10&&mode<=16){
  AircraftComparisonScene scene(r,3);scene.production(1,4,false);
  static constexpr float banks[]{0,-.13962634f,-.27925268f,-.41887902f,.13962634f,.27925268f,.41887902f};
  static constexpr Vec2 positions[]{{-150,140},{-150,-140},{150,140},{150,-140}};
  unsigned index=0;for(const auto& a:scene.world->actors())if(!a->removed&&a->flightTactics){
   a->position=a->aircraft.position=positions[index++];a->angle=a->aircraft.angle=1.57079632679f;
   a->flightTactics->bank=banks[mode-10];a->radarAngle=0;
  }
  auto reference=scene.world->validationSpawn("E_Tank_32",false,{0,0});
  scene.world->actor(reference)->weapons.clear();
  return scene.frame();
 }
 auto models=BattleModels::load(r);appendAircraftModels(models);
 auto dims=[&](std::string_view n){return r.imageSize(n);};
 Frame f;f.logicalWidth=640;f.logicalHeight=800;f.background={.18f,.20f,.20f,1};
 auto one=[&](std::string_view image,Vec2 center,float scale){scale*=aircraftAssemblyScale(image.substr(0,image.size()-4));auto d=dims(image);Sprite s;s.image=image;if(auto part=models.part(image.substr(0,image.size()-4))){d.x/=part->splitX;d.y/=part->splitY;s.sourcePx=Rect{0,0,d.x,d.y};}s.destination={center.x-d.x*scale/2,center.y-d.y*scale/2,d.x*scale,d.y*scale};s.rotationDegrees=-90;s.nearest=true;f.sprites.push_back(s);};
 auto machine=[&](const char* name,Vec2 c,float scale){Frame t;models.appendSprites(t,name,c,scale,1.57079632679f,1.57079632679f,mode>=4?0.f:float(mode)*.5f,dims);for(auto s:t.sprites)if(s.color.r!=0){s.nearest=true;f.sprites.push_back(s);}};
 if(mode==7){
  f.logicalWidth=900;f.logicalHeight=900;
  // The web review places the preserved prior-game capture in the empty
  // top-left cell. All other cells use the current production renderer.
  machine("AircraftLight",{450,150},2);machine("AircraftDuct",{750,150},2);
  // Middle-left is the prior Scout BODY capture. Middle-center is current
  // BODY only: compare its opaque bounds separately from the unchanged rotor.
  one("ALightBody.png",{450,450},2);
  const float partScale=2*aircraftAssemblyScale("ALightBody");
  one("ALightRotor.png",{750,450},partScale);
  machine("AircraftHeavy",{150,750},2);machine("AircraftTransport",{450,750},2);
  auto table=[&](const char* n){auto b=r.read(std::string("Tables/")+n+".tsv");return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});};
  auto builds=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));
  auto player=PlayerLoadout::fromBuild(*builds.find("MachineGun_10"),table("ItemInfo"),BattleGeometry::pad());
  auto poses=models.pose(player.assembly,2,1.57079632679f,1.57079632679f,0);
  Frame t;models.appendSprites(t,poses,{750,750},dims);for(auto s:t.sprites)if(s.color.r!=0){s.nearest=true;f.sprites.push_back(s);}
  return f;
 }
 if(mode==4){
  f.logicalWidth=900;f.logicalHeight=900;
  // Left: 1b3afed size. Right: widths matched to original reference units.
  // Cancelling only the documented model multiplier reconstructs the baseline.
  machine("AircraftHeavy",{225,150},2*1.32f/aircraftAssemblyScale("AHeavyBody"));
  machine("AircraftHeavy",{675,150},2);
  machine("AircraftTransport",{225,450},2*.765f/aircraftAssemblyScale("ATransportBody"));
  machine("AircraftTransport",{675,450},2);
  machine("AircraftDuct",{225,750},2*.765f/aircraftAssemblyScale("ADuctBody"));
  machine("AircraftDuct",{675,750},2);
  return f;
 }
 if(mode==6){
  f.logicalWidth=900;f.logicalHeight=600;
  machine("AircraftLight",{150,150},2/aircraftAssemblyScale("ALightBody"));
  machine("AircraftLight",{450,150},2);
  machine("AircraftDuct",{750,150},2);
  machine("AircraftHeavy",{150,450},2);
  machine("AircraftTransport",{450,450},2);
  machine("AircraftDuct",{750,450},2);
  return f;
 }
 if(mode==5){
  f.logicalWidth=900;f.logicalHeight=900;
  auto table=[&](const char* n){auto b=r.read(std::string("Tables/")+n+".tsv");return DataTable::parse({reinterpret_cast<const char*>(b.data()),b.size()});};
  auto builds=BuildCatalog::load(table("MachBuild_Parts"),table("MachBuild_Name"));
  auto player=PlayerLoadout::fromBuild(*builds.find("MachineGun_10"),table("ItemInfo"),BattleGeometry::pad());
  for(unsigned row=0;row<3;++row){
   const float y=150+300*row;
   machine("E_Tank_32",{150,y},2);
   auto poses=models.pose(player.assembly,2,1.57079632679f,1.57079632679f,0);
   Frame t;models.appendSprites(t,poses,{450,y},dims);
   for(auto s:t.sprites)if(s.color.r!=0){s.nearest=true;f.sprites.push_back(s);}
   machine(row==0?"AircraftHeavy":row==1?"AircraftTransport":"AircraftDuct",{750,y},2);
  }
  return f;
 }
 if(mode==3){
  f.logicalWidth=900;f.logicalHeight=900;
  // Each row: original game reference, generated fixed body, runtime assembly.
  // All three columns use the same 2x world scale, without fitting each image.
  machine("E_Tank_28",{150,150},2);one("AHeavyBody.png",{450,150},2);machine("AircraftHeavy",{750,150},2);
  one("P_Body_34.png",{150,450},2);one("ATransportBody.png",{450,450},2);machine("AircraftTransport",{750,450},2);
  one("P_Body_13.png",{150,750},2);one("ADuctBody.png",{450,750},2);machine("AircraftDuct",{750,750},2);
  return f;
 }
 if(mode==2){
  one("P_Body_18.png",{160,140},2);one("ATransportBody.png",{450,180},2);
  machine("E_Tank_28",{160,470},2);machine("AircraftTransport",{450,470},2);
  machine("AircraftTransport",{160,710},1);machine("AircraftDuct",{450,710},1);
  return f;
 }
 // All rows use 2x original logical pixels. No per-image fit-to-cell scaling.
 one("P_Body_18.png",{160,110},2);one("ALightBody.png",{460,110},2);
 machine("E_Tank_28",{160,380},2);one("AHeavyBody.png",{460,380},2);
 machine("E_Turret_18",{160,655},2);machine("AircraftHeavy",{460,655},2);
 return f;
}
}
