#include "UpgradeDialog.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace md;
struct Resources:ResourceAccess{
 std::filesystem::path root;std::map<std::string,Vec2> sizes;explicit Resources(std::filesystem::path p):root(std::move(p)){}
 Bytes read(std::string_view p)override{std::ifstream f(root/std::string(p),std::ios::binary);assert(f);return {std::istreambuf_iterator<char>(f),{}};}
 Vec2 imageSize(std::string_view n)override{auto key=std::string(n);if(sizes.contains(key))return sizes.at(key);auto b=read("Images/"+key);auto u=[&](int i){return std::uint32_t(b[i])<<24|std::uint32_t(b[i+1])<<16|std::uint32_t(b[i+2])<<8|b[i+3];};return sizes[key]={float(u(16)),float(u(20))};}
};
void tap(UpgradeDialog& d,float x,float y){d.touch({1,TouchPhase::began,{x,y}});d.touch({1,TouchPhase::ended,{x,y}});}
void validate(Frame& f,Resources& r){for(auto& s:f.sprites){assert(std::filesystem::exists(s.image.starts_with("Generated/")?r.root/s.image:r.root/"Images"/s.image));if(s.sourcePx&&!s.image.starts_with("Generated/")){auto z=r.imageSize(s.image);assert(s.sourcePx->x+s.sourcePx->width<=z.x&&s.sourcePx->y+s.sourcePx->height<=z.y);}}}
int main(int argc,char** argv){
 Resources r(argc>1?argv[1]:"Resources");unsigned models=0;UpgradeDialog d(r,[&](Frame&,const UpgradeUnit&,bool,Vec2,float s){assert(s==.5f);++models;});
 assert(!d.touch({1,TouchPhase::began,{0,0}}));
 UpgradeDialogState state;state.kind=UpgradeDialogState::Kind::base;for(auto& row:state.base)row={1,9,125,true,""};d.setState(state);
 tap(d,132,456);assert(d.takeActions().empty()); // upgrade requires category selection.
 tap(d,196,251);auto a=d.takeActions();assert(a.size()==1&&a[0].buttonId==0x407&&a[0].row==0);
 tap(d,132,456);a=d.takeActions();assert(a.size()==1&&a[0].buttonId==0x408&&a[0].row==0);
 state.base[0].enabled=false;d.setState(state);tap(d,132,456);assert(d.takeActions().empty());
 tap(d,256,456);assert(d.takeActions().at(0).buttonId==0x413);
 Frame f;d.append(f);validate(f,r);
 state.kind=UpgradeDialogState::Kind::units;UpgradeUnit u;u.name="MachineGun";u.englishName="Machine Gun";u.cost=300;u.maximumLevel=10;u.canUpgrade=true;u.canInstall=true;u.hasNext=true;u.stats={10,150,80,2};u.nextStats={12,160,90,2};state.units.push_back(u);d.setState(state);
 // A world update between down/up must not lose the held button.
 d.touch({5,TouchPhase::began,{32,70.25f}});d.setState(state);d.touch({5,TouchPhase::ended,{32,70.25f}});
 a=d.takeActions();assert(a.size()==1&&a[0].buttonId==0x411&&a[0].row==0);
 f={};d.append(f);validate(f,r);bool regular=false;for(const auto& s:f.sprites)regular|=s.image=="Generated/TrebuchetMS-48.png";assert(regular&&models==3);
 tap(d,198,296.75f);a=d.takeActions();assert(a.size()==1&&a[0].buttonId==0x400);
 tap(d,198,325.75f);assert(d.takeActions().at(0).buttonId==0x401);
 state.selectedUnit=0;state.units[0].installed=true;state.units[0].canUpgrade=false;d.setState(state);
 tap(d,198,296.75f);assert(d.takeActions().empty());tap(d,198,325.75f);assert(d.takeActions().at(0).buttonId==0x402);
 tap(d,192,353.75f);assert(d.takeActions().at(0).buttonId==0x3ff);
 state.selectedUnit=999;d.setState(state);f={};d.append(f);validate(f,r);
 state.kind=UpgradeDialogState::Kind::attacks;UpgradeAttack attack;attack.name="Air Strike";attack.icon="Icon02001.png";attack.nextIcon="Icon02002.png";attack.maximumLevel=10;attack.buyCost=70;attack.upgradeCost=400;attack.count=3;attack.canBuy=true;attack.canUpgrade=true;attack.canInstall=true;attack.hasNext=true;state.attacks.push_back(attack);d.setState(state);
 tap(d,60,70.25f);a=d.takeActions();assert(a.size()==1&&a[0].buttonId==0x412);
 f={};d.append(f);validate(f,r);
 tap(d,226,264.75f);assert(d.takeActions().at(0).buttonId==0x404);
 tap(d,226,296.75f);assert(d.takeActions().at(0).buttonId==0x403);
 tap(d,226,325.75f);assert(d.takeActions().at(0).buttonId==0x405);
 state.selectedAttack=0;state.attacks[0].installed=true;state.attacks[0].canUpgrade=false;state.attacks[0].canBuy=false;d.setState(state);
 tap(d,226,264.75f);assert(d.takeActions().empty());tap(d,226,296.75f);assert(d.takeActions().empty());
 tap(d,226,325.75f);assert(d.takeActions().at(0).buttonId==0x406);
 tap(d,220,353.75f);assert(d.takeActions().at(0).buttonId==0x3ff);
 state.selectedAttack=999;d.setState(state);f={};d.append(f);validate(f,r);
 assert(!d.touch({91,TouchPhase::began,{292,52}}));assert(!d.touch({91,TouchPhase::ended,{292,52}}));
 assert(!d.touch({92,TouchPhase::began,{161,443}}));assert(d.touch({93,TouchPhase::began,{150,330}}));
 std::cout<<"UpgradeDialog original base and unit actions, blocked upgrades, glyphs and asset bounds passed\n";
}
