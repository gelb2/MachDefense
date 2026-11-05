#include "../Modules/SaveTransfer/Codec.hpp"
#include "SessionTestSupport.hpp"
#include <random>
int main(int argc,char**argv){
 auto s=SaveData::empty(123);for(unsigned i=0;i<5;++i){s.createProfile("PROFILE"+std::to_string(i),10+i);s.setCredits(i,100+i);s.writeWord(SaveData::slotStart+i*SaveData::slotStride+0xc,i*10);}
 auto code=transfer::encode(s);assert(transfer::decode(code).encode()==s.encode());assert(transfer::encode(transfer::decode(code))==code);
 auto reject=[](std::string c){try{transfer::decode(c);return false;}catch(...){return true;}};
 assert(reject("HM1-abc"));assert(reject(code.substr(0,code.size()-8)));assert(reject(std::string(120001,'A')));auto bad=code;bad[12]=bad[12]=='A'?'B':'A';assert(reject(bad));
 std::mt19937 rng(123);for(int i=0;i<400;++i){auto b=code;auto p=4+rng()%(b.size()-4);b[p]=transfer::alphabet[(rng()%63+1+(std::find(transfer::alphabet,transfer::alphabet+64,b[p])-transfer::alphabet))%64];assert(reject(b));}
 if(argc==3){std::ifstream f(argv[1],std::ios::binary);assert(f);Bytes b{std::istreambuf_iterator<char>(f),{}};auto original=SaveData::decode(b);auto c=transfer::encode(original);assert(transfer::decode(c).encode()==b);std::ofstream(argv[2])<<c;
 std::cout<<"phone bytes="<<b.size()<<" code chars="<<c.size()<<"\n";for(size_t i=0;i<5;++i)if(original.active(i))std::cout<<"slot="<<i<<" stage="<<original.stage(i)<<" credits="<<original.credits(i)<<" builds="<<original.builds(i).size()<<"\n";}
 std::cout<<"SaveTransfer: five slots, exact bytes, deterministic code, 403 malformed inputs rejected\n";
}
