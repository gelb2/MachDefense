#include "Core/Presentation/UpgradeDialog.hpp"
#include <cassert>
#include <algorithm>
#include <cmath>
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
 // Full catalog: preserve 64px cards, grow on tall screens, otherwise scroll
 // only the clipped list. Detail/action buttons never move during scrolling.
 UpgradeDialog catalog(r);UpgradeDialogState all;all.kind=UpgradeDialogState::Kind::units;
 for(unsigned i=0;i<18;++i){auto entry=u;entry.name="Unit"+std::to_string(i);all.units.push_back(entry);}all.selectedUnit=0;
 auto find=[](const Frame& frame,std::string_view name)->const Sprite&{auto it=std::find_if(frame.sprites.begin(),frame.sprites.end(),[&](const auto& sprite){return sprite.image==name;});assert(it!=frame.sprites.end());return *it;};
 auto cardCount=[](const Frame& frame){return std::count_if(frame.sprites.begin(),frame.sprites.end(),[](const auto& sprite){return sprite.image=="Btn_MachBuild.png"&&sprite.sourcePx->x==0;});};
 for(const auto size:std::array<Vec2,4>{{{320,480},{390,844},{844,390},{320,568}}}){
  FrameLayout layout;layout.resize(size.x,size.y);catalog.setLayout(layout);catalog.setState(all);
  Frame before;catalog.append(before);validate(before,r);
  const auto first=find(before,"Btn_MachBuild.png");assert(first.clip&&first.destination.width==64&&first.destination.height==64);
  const auto clip=*first.clip;const bool tall=layout.height-layout.top-layout.bottom>=562;
  assert(cardCount(before)==(tall?18:15));assert(clip.height==(tall?250:168));
  const auto detail=find(before,"UpgradeMachBG.png").destination;
  assert(std::abs(detail.y-(clip.y+clip.height+7.5f))<.001f);
  for(const auto& sprite:before.sprites)if(sprite.clip){assert(sprite.clip->y==clip.y&&sprite.clip->height==clip.height);}
  assert(!catalog.scrollUnits({198,detail.y+83},500));
  assert(catalog.scrollUnits({32,clip.y+32},10000));Frame after;catalog.append(after);
  assert(find(after,"UpgradeMachBG.png").destination.y==detail.y);
  const float lastY=clip.y+32+186-(tall?0:82);
  tap(catalog,124,lastY);auto selected=catalog.takeActions();assert(selected.size()==1&&selected[0].buttonId==0x411&&selected[0].row==17);
  assert(catalog.scrollUnits({32,clip.y+32},-10000));
  // A list drag ending on an upgrade button must not select or buy anything.
  catalog.touch({20,TouchPhase::began,{32,clip.y+32}});
  catalog.touch({20,TouchPhase::moved,{198,detail.y+83}});
  catalog.touch({21,TouchPhase::began,{198,detail.y+83}});
  catalog.touch({21,TouchPhase::ended,{198,detail.y+83}});
  catalog.touch({20,TouchPhase::ended,{198,detail.y+83}});assert(catalog.takeActions().empty());
  // Scrolling drag survives setState refreshes; release is not a card tap.
  catalog.touch({22,TouchPhase::began,{124,clip.y+120}});catalog.setState(all);
  catalog.touch({22,TouchPhase::moved,{124,clip.y+20}});catalog.setState(all);
  catalog.touch({22,TouchPhase::ended,{124,clip.y+20}});assert(catalog.takeActions().empty());
  // Rotation cancels the held card, including its eventual release.
  catalog.touch({23,TouchPhase::began,{124,clip.y+32}});catalog.setLayout(layout);
  catalog.touch({23,TouchPhase::ended,{124,clip.y+32}});assert(catalog.takeActions().empty());
 }
 // Safe areas shift art and hit boxes together in session-local coordinates.
 FrameLayout safe;safe.resize(390,844,44,34);catalog.setLayout(safe);catalog.setState(all);Frame safeFrame;catalog.append(safeFrame);
 const auto firstSafe=find(safeFrame,"Btn_MachBuild.png");assert(cardCount(safeFrame)==18);
 assert(std::abs(firstSafe.clip->y+safe.center().y-(safe.top+38.25f))<.001f);
 tap(catalog,32,firstSafe.destination.y+32);assert(catalog.takeActions().at(0).row==0);
 std::cout<<"UpgradeDialog original base and unit actions, blocked upgrades, glyphs and asset bounds passed\n";
}
