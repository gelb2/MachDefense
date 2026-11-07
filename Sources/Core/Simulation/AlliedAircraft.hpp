#pragma once
#include <array>
#include <string_view>
namespace md {
struct AlliedAircraftDefinition {std::string_view name,model,label;unsigned mission;float widthFraction,height; float noseDegreesPerSecond,weaponDegreesPerSecond;};
inline constexpr std::array<AlliedAircraftDefinition,4> alliedAircraftCatalog{{
 {"ScoutHeli","AircraftLight","Scout Heli",8,.50f,12,100,180},
 {"Gunship","AircraftHeavy","Gunship",36,.425f,18,45,90},
 {"Orca","AircraftDuct","Orca",16,.425f,14,80,150},
 {"OrcaBomber","AircraftTransport","Orca Bomber",24,.25f,16,60,120}
}};
inline const AlliedAircraftDefinition* alliedAircraftForModel(std::string_view model){
 for(const auto& a:alliedAircraftCatalog)if(model.starts_with(a.model)&&(model.size()==a.model.size()+3||model==a.model))return &a;
 return nullptr;
}
}
