#pragma once
#include "Core/Simulation/Battle.hpp"
#include "Core/Simulation/AircraftModels.hpp"
#include "Core/Simulation/PlayerParts.hpp"
#include <algorithm>
namespace md::validation {
// Cheat-only measured sheet: production parts and sprite assembly, uniform scale.
inline Frame aircraftVisualFrame(ResourceAccess& r,unsigned mode){
 auto models=BattleModels::load(r);appendAircraftModels(models);
 auto dims=[&](std::string_view n){return r.imageSize(n);};
 Frame f;f.logicalWidth=640;f.logicalHeight=800;f.background={.18f,.20f,.20f,1};
 auto one=[&](std::string_view image,Vec2 center,float scale){scale*=aircraftAssemblyScale(image.substr(0,image.size()-4));auto d=dims(image);Sprite s;s.image=image;s.destination={center.x-d.x*scale/2,center.y-d.y*scale/2,d.x*scale,d.y*scale};s.rotationDegrees=-90;s.nearest=true;f.sprites.push_back(s);};
 auto machine=[&](const char* name,Vec2 c,float scale){Frame t;models.appendSprites(t,name,c,scale,1.57079632679f,1.57079632679f,mode>=4?0.f:float(mode)*.5f,dims);for(auto s:t.sprites)if(s.color.r!=0){s.nearest=true;f.sprites.push_back(s);}};
 if(mode==4){
  f.logicalWidth=900;f.logicalHeight=900;
  // Left: b01d751 size. Right: additional +10% / -10%. Same PNGs and pose.
  // Cancelling only the documented model multiplier reconstructs the baseline.
  machine("AircraftHeavy",{225,150},2*1.20f/aircraftAssemblyScale("AHeavyBody"));
  machine("AircraftHeavy",{675,150},2);
  machine("AircraftTransport",{225,450},2*.85f/aircraftAssemblyScale("ATransportBody"));
  machine("AircraftTransport",{675,450},2);
  machine("AircraftDuct",{225,750},2*.85f/aircraftAssemblyScale("ADuctBody"));
  machine("AircraftDuct",{675,750},2);
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
