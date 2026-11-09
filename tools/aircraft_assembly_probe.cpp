#include "../Tests/PNGTestSize.hpp"
// Offline art inspection: consumes the production assembly and sprite routines.
// No gameplay, campaign, save or platform code is changed by this executable.
#include "Core/Simulation/Battle.hpp"
#include <cassert>
#include <bit>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
namespace fs=std::filesystem;
std::vector<std::uint8_t> read(const fs::path& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error(p.string());return {std::istreambuf_iterator<char>(f),{}};}
md::Chunk named(unsigned type,const char* name){md::Chunk c;c.type=type;c.data.resize(24);std::string n=name;assert(n.size()<24);std::copy(n.begin(),n.end(),c.data.begin());return c;}
void real(md::Chunk& c,float v){auto u=std::bit_cast<std::uint32_t>(v);for(int i=0;i<4;++i)c.data.push_back((u>>(8*i))&255);}
md::Chunk part(const char* name,unsigned type,std::initializer_list<md::Vec2> sockets){auto c=named(1,name);c.data.push_back(type);c.data.push_back(1);c.data.push_back(sockets.size());for(auto s:sockets){real(c,s.x);real(c,s.y);}return c;}
md::Chunk item(const char* name,unsigned socket,unsigned z){auto c=named(4,name);c.data.push_back(socket);c.data.push_back(z);return c;}
md::Vec2 imageSize(const fs::path& p){auto b=read(p);return mdtest::pngSize(b);}
void output(const md::Frame& f){bool first=true;std::cout<<"[";for(const auto& s:f.sprites){if(!first)std::cout<<",";first=false;auto d=s.destination;auto r=*s.sourcePx;std::cout<<"{\"image\":\""<<s.image<<"\",\"destination\":["<<d.x<<","<<d.y<<","<<d.width<<","<<d.height<<"],\"source\":["<<r.x<<","<<r.y<<","<<r.width<<","<<r.height<<"],\"rotation\":"<<s.rotationDegrees<<",\"shadow\":"<<(s.color.r==0?"true":"false")<<"}";}std::cout<<"]";}
int main(int argc,char** argv){
 assert(argc==2);fs::path root=argv[1];auto atlas=root/"Art/Aircraft/parts/aircraft-parts-v2.png";auto size=imageSize(atlas);assert(size.x==1254&&size.y==1254);
 auto models=md::BattleModels::parse(md::Chunk::parse(read(root/"Resources/RawData/PartsInfo.mcb")),md::Chunk::parse(read(root/"Resources/RawData/MachInfo.mcb")));
 md::Chunk parts;parts.type=0;
 // Socket positions are in atlas-cell pixels before the single unit scale.
 // Rotor anchor includes the existing visualScale, so its painted hub meets socket 1/2.
 parts.children={part("ALightBody",0,{{0,0},{2,66}}),part("ALightRotor",7,{{0,0}}),part("AHeavyBody",0,{{0,0},{-202,65},{213,65}}),part("AHeavyRotor",7,{{0,6}})};
 md::Chunk machines;machines.type=2;
 auto light=named(3,"AircraftLight");auto lb=item("ALightBody",0,1);lb.children={item("ALightRotor",1,2)};light.children={lb};
 auto heavy=named(3,"AircraftHeavy");auto hb=item("AHeavyBody",0,1);hb.children={item("AHeavyRotor",1,2),item("AHeavyRotor",2,2)};heavy.children={hb};machines.children={light,heavy};models.append(parts,machines);
 models.configureTiles(md::DataTable::parse("PartsName\tType\tNo\tTileName\nALightBody\t0\t0\taircraft-parts-v2\nALightRotor\t7\t1\taircraft-parts-v2\nAHeavyBody\t0\t2\taircraft-parts-v2\nAHeavyRotor\t7\t3\taircraft-parts-v2\n"),md::DataTable::parse("TileName\tX\tY\naircraft-parts-v2.png\t2\t2\n"));
 auto dimensions=[&](std::string_view n){return imageSize(n=="aircraft-parts-v2.png"?atlas:root/"Resources/Images"/n);};
 std::cout<<"{\"references\":[";bool first=true;
 for(const char* name:{"E_Tank_01","E_Tank_32"}){if(!first)std::cout<<",";first=false;md::Frame f;models.appendSprites(f,name,{},1,1.57079632679f,1.57079632679f,0,dimensions);std::cout<<"{\"name\":\""<<name<<"\",\"sprites\":";output(f);std::cout<<"}";}
 std::cout<<"],\"aircraft\":[";first=true;
 for(const char* name:{"AircraftLight","AircraftHeavy"}){if(!first)std::cout<<",";first=false;bool small=std::string_view(name)=="AircraftLight";float scale=small?.09913f:.137f;auto tree=*models.machine(name);for(auto& rotor:tree[0].children)rotor.visualScale=.55f;
  std::cout<<"{\"name\":\""<<name<<"\",\"scale\":"<<scale<<",\"frames\":[";
  for(int phase=0;phase<72;++phase){if(phase)std::cout<<",";float angle=float(phase)*6.28318530718f/72;auto poses=models.pose(tree,scale,0,0,angle);assert(poses.size()==(small?2:3));for(auto& p:poses){assert(std::isfinite(p.localCenter.x)&&std::isfinite(p.localCenter.y));}md::Frame f;models.appendSprites(f,poses,{},dimensions);output(f);}
  std::cout<<"]}";
 }
 std::cout<<"]}\n";
}
