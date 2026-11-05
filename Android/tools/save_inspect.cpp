#include "Core/Session/SaveData.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
int main(int argc,char**argv){try{
 if(argc!=2)return 2;
 std::ifstream in(argv[1],std::ios::binary);if(!in)throw std::runtime_error("Cannot open save");
 std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});
 auto s=md::SaveData::decode(bytes);std::cout<<"{\"bytes\":"<<bytes.size()<<",\"slots\":[";
 for(unsigned i=0;i<5;++i){if(i)std::cout<<',';std::cout<<"{\"active\":"<<(s.active(i)?"true":"false");
 if(s.active(i)){
 std::cout<<",\"name\":"<<std::quoted(s.name(i))<<",\"stage\":"<<s.stage(i)<<",\"credits\":"<<s.credits(i)<<",\"score\":"<<s.readWord(s.slotStart+i*s.slotStride+0x2054)<<",\"base\":[";
 for(unsigned j=0;j<6;++j){if(j)std::cout<<',';std::cout<<s.baseUpgrade(i,j);}std::cout<<"],\"builds\":[";
 bool comma=false;for(auto&b:s.builds(i)){if(comma)std::cout<<',';comma=true;std::cout<<"{\"name\":"<<std::quoted(b.name)<<",\"level\":"<<b.level<<",\"onSlot\":"<<(b.onSlot?"true":"false")<<'}';}std::cout<<"],\"attacks\":[";
 comma=false;for(auto&a:s.attacks(i)){if(comma)std::cout<<',';comma=true;std::cout<<"{\"name\":"<<std::quoted(a.name)<<",\"level\":"<<a.level<<",\"count\":"<<a.count<<",\"onSlot\":"<<(a.onSlot?"true":"false")<<'}';}std::cout<<"],\"clears\":[";
 for(unsigned j=0;j<80;++j){if(j)std::cout<<',';std::cout<<s.clearCount(i,j);}std::cout<<']';
 }std::cout<<'}';}std::cout<<"]}\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
