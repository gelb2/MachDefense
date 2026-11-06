#include "SessionTestSupport.hpp"
#include "../Modules/SaveTransfer/Codec.hpp"
struct TransferHost:Host {
 std::string field,copied;bool opened=false,readOnly=false,fail=false;int opens=0,layouts=0;Rect bounds{};
 bool supportsSaveTransfer()const override{return true;}
 void openTransferField(std::string text,bool ro,Rect r)override{field=text;readOnly=ro;bounds=r;opened=true;++opens;}
 void layoutTransferField(Rect r)override{bounds=r;++layouts;}
 std::string transferFieldText()const override{return field;}
 void closeTransferField()override{opened=false;field.clear();}
 void copyTransferCode(std::string text)override{copied=text;}
 bool writeSave(std::span<const uint8_t> b)override{return fail?false:Host::writeSave(b);}
};
int main(int argc,char**argv){assert(argc==2);Resources r;r.root=argv[1];TransferHost h;auto old=SaveData::empty(1);old.createProfile("OLD",10);h.saved=old.encode();GameSession g(r,h,false);tap(g,160,240);ticks(g);
 auto fresh=SaveData::empty(9);fresh.createProfile("NEW",12);fresh.setCredits(0,1234);const auto code=transfer::encode(fresh);
 auto enter=[&](const std::string& text){tap(g,235,72);assert(h.opened&&!h.readOnly);h.field=text;tap(g,85,71);assert(!h.opened);};
 tap(g,85,72);assert(h.opened&&h.readOnly);assert(transfer::decode(h.field).encode()==old.encode());tap(g,85,71);assert(h.copied==h.field);assert(h.writes==0);tap(g,235,71);assert(!h.opened);
 tap(g,235,72);h.field=code;tap(g,235,71);assert(!h.opened&&h.writes==0);
 enter(code);tap(g,208,295);assert(h.saved==old.encode());assert(h.writes==0);
 enter(code);h.fail=true;tap(g,112,295);assert(h.saved==old.encode());assert(h.writes==0);tap(g,160,295);h.fail=false;
 enter(code);tap(g,112,295);assert(h.saved==fresh.encode());assert(h.writes==1);tap(g,160,295);tap(g,85,72);assert(h.field==code);tap(g,235,71);
 enter("MD1-invalid");assert(h.writes==1);tap(g,160,295);
 auto invalid=fresh;invalid.setBaseUpgrade(0,0,255);enter(transfer::encode(invalid));tap(g,160,295);assert(h.writes==1&&h.saved==fresh.encode());
 auto badUnit=fresh;auto units=badUnit.builds(0);units[0].name="NotAUnit";badUnit.setBuilds(0,units);enter(transfer::encode(badUnit));tap(g,160,295);assert(h.writes==1&&h.saved==fresh.encode());
 auto complete=fresh;complete.writeWord(SaveData::slotStart+0x38,80);enter(transfer::encode(complete));tap(g,112,295);assert(h.saved==complete.encode()&&h.writes==2);tap(g,160,295);
 tap(g,235,72);h.field=code;const int layouts=h.layouts;g.resize(390,844,0,0);assert(h.opened&&h.field==code&&h.layouts>layouts);checkImages(g.frame(),r);ticks(g,100);assert(h.writes==2);assert(h.field==code);g.resize(320,480,0,0);tap(g,235,71);
 std::cout<<"Transfer menu: game-asset actions, copy/cancel/failed write/restore, invalid input, resize, no frame writes passed\n";
}
