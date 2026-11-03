#include "PlayerParts.hpp"
#include <cstdio>
#include <stdexcept>
namespace md {
namespace {
std::string numbered(std::string_view prefix,unsigned number){char suffix[16];std::snprintf(suffix,sizeof suffix,"%02u",number);return std::string(prefix)+suffix;}
AssemblyPart node(std::string name,unsigned socket,unsigned z){AssemblyPart p;p.name=std::move(name);p.parentSocket=std::uint8_t(socket);p.zOrder=std::uint8_t(z);return p;}
std::size_t itemRow(const DataTable& items,int id){
    char key[16];std::snprintf(key,sizeof key,"%05d",id);auto row=items.findRow(key);
    if(!row)throw std::runtime_error("Unknown original equipment code "+std::string(key));return *row;
}
float parameter(const DataTable& items,std::size_t row){auto n=items.number(row,"Param1");if(!n)throw std::runtime_error("Missing equipment parameter");return *n;}
}
PlayerLoadout PlayerLoadout::fromBuild(const BuildDefinition& build,const DataTable& items,BattleGeometry geometry){
    if(build.setType!=0)throw std::invalid_argument("Named units use MachInfo assemblies");
    const auto armor=itemRow(items,build.armor),foot=itemRow(items,build.foot),weapon=itemRow(items,build.weapon);
    PlayerLoadout result;result.hp=int(parameter(items,armor));result.stepSize=parameter(items,foot)*geometry.deviceScale;
    result.radius=float(build.radius)*geometry.deviceScale;result.weapon=items.value(weapon,"strParam");
    auto root=node("DummyParts",0,0);
    root.children.push_back(node(std::string(items.value(foot,"strParam")),1,0));
    root.children.push_back(node(std::string(items.value(foot,"strParam")),2,0));
    auto body=node(std::string(items.value(armor,"strParam")),3,0);
    body.children.push_back(node(result.weapon,1,0));
    body.children.push_back(node(result.weapon,2,0));
    body.children.back().reverse=2;
    root.children.push_back(std::move(body));result.assembly.push_back(std::move(root));return result;
}
PlayerLoadout PlayerLoadout::forBase(const SaveData& save,std::size_t slot,const BaseUpgradeCatalog& upgrades,BattleGeometry geometry){
    const auto defense=save.baseUpgrade(slot,unsigned(BaseUpgrade::defense)),cannon=save.baseUpgrade(slot,unsigned(BaseUpgrade::cannon));
    PlayerLoadout result;result.radius=64*geometry.deviceScale;
    const auto hp=upgrades.level(BaseUpgrade::defense,defense).parameter;if(!hp)throw std::runtime_error("Missing base HP");result.hp=int(*hp);
    result.weapon=numbered("BaseWpn",cannon+1);
    auto root=node("DummyParts",0,0),body=node(numbered("Base",defense+1),3,0);
    body.children.push_back(node(result.weapon,1,0));body.children.push_back(node(result.weapon,2,0));
    body.children.back().reverse=2;
    body.children.push_back(node(numbered("Base",cannon+1)+"_Rader",3,0));
    root.children.push_back(std::move(body));result.assembly.push_back(std::move(root));return result;
}
}
