#include "SessionTestSupport.hpp"
#include "../Modules/SaveTransfer/Codec.hpp"
struct TransferHost:Host {
 std::optional<std::string> supplied;std::string exported;bool fail=false;
 bool supportsSaveTransfer()const override{return true;}
 void transferText(std::string initial,bool exporting,std::function<void(std::optional<std::string>)> cb)override{if(exporting){exported=initial;cb({});}else cb(supplied);}
 bool writeSave(std::span<const uint8_t> b)override{return fail?false:Host::writeSave(b);}
};
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];TransferHost h;auto old=SaveData::empty(1);old.createProfile("OLD",10);h.saved=old.encode();GameSession g(r,h,false);tap(g,160,240);ticks(g);
 auto fresh=SaveData::empty(9);fresh.createProfile("NEW",12);fresh.setCredits(0,1234);h.supplied=transfer::encode(fresh);
 tap(g,85,72);assert(transfer::decode(h.exported).encode()==old.encode());assert(h.writes==0);
 tap(g,235,72);tap(g,208,295);assert(h.saved==old.encode());assert(h.writes==0);
 tap(g,235,72);h.fail=true;tap(g,112,295);assert(h.saved==old.encode());assert(h.writes==0);tap(g,160,295);h.fail=false;
 tap(g,235,72);tap(g,112,295);assert(h.saved==fresh.encode());assert(h.writes==1);tap(g,160,295);tap(g,85,72);assert(h.exported==*h.supplied);
 h.supplied="MD1-invalid";tap(g,235,72);assert(h.writes==1);tap(g,160,295);
 auto invalid=fresh;invalid.setBaseUpgrade(0,0,255);h.supplied=transfer::encode(invalid);tap(g,235,72);tap(g,160,295);assert(h.writes==1&&h.saved==fresh.encode());
 auto badUnit=fresh;auto units=badUnit.builds(0);units[0].name="NotAUnit";badUnit.setBuilds(0,units);h.supplied=transfer::encode(badUnit);tap(g,235,72);tap(g,160,295);assert(h.writes==1&&h.saved==fresh.encode());
 ticks(g,100);assert(h.writes==1);std::cout<<"Transfer menu: export/cancel/failed write/restore/re-export/malformed input; normal game UI passed\n";
}
