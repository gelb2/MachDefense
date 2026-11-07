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
md::Vec2 imageSize(const fs::path& p){auto b=read(p);auto be=[&](int x){return unsigned(b[x])<<24|unsigned(b[x+1])<<16|unsigned(b[x+2])<<8|unsigned(b[x+3]);};return {float(be(16)),float(be(20))};}
void output(const md::Frame& f){bool first=true;std::cout<<"[";for(const auto& s:f.sprites){if(!first)std::cout<<",";first=false;auto d=s.destination;auto r=*s.sourcePx;std::cout<<"{\"image\":\""<<s.image<<"\",\"destination\":["<<d.x<<","<<d.y<<","<<d.width<<","<<d.height<<"],\"source\":["<<r.x<<","<<r.y<<","<<r.width<<","<<r.height<<"],\"rotation\":"<<s.rotationDegrees<<",\"shadow\":"<<(s.color.r==0?"true":"false")<<"}";}std::cout<<"]";}

#include "Core/Validation/AircraftComparison.hpp"
int main(int argc,char** argv){assert(argc==2);fs::path root=argv[1];auto models=md::BattleModels::parse(md::Chunk::parse(read(root/"Resources/RawData/PartsInfo.mcb")),md::Chunk::parse(read(root/"Resources/RawData/MachInfo.mcb")));md::validation::appendAircraftModels(models);auto dimensions=[&](std::string_view n){return imageSize(root/"Resources/Images"/n);};
 std::cout<<"{\"aircraft\":[";bool first=true;
 for(const char* name:{"AircraftLight","AircraftHeavy"}){if(!first)std::cout<<",";first=false;std::cout<<"{\"name\":\""<<name<<"\",\"scale\":1,\"frames\":[";for(int phase=0;phase<360;++phase){if(phase)std::cout<<",";md::Frame f;models.appendSprites(f,name,{},1,1.57079632679f,1.57079632679f,float(phase)*6.28318530718f/360,dimensions);output(f);}std::cout<<"]}";}
 std::cout<<"]}";
}
