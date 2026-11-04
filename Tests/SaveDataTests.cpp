#include "Core/Contracts/DarwinRandom.hpp"
#include "Core/Session/SaveData.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

void check(bool condition,const char* label) { if(!condition) throw std::runtime_error(label); }
template<class F> void rejects(F f,const char* label) { bool bad=false;try{f();}catch(const std::exception&){bad=true;}check(bad,label); }
int main(int argc,char**argv) {
 try {
    constexpr std::array<std::uint32_t,5> seeds{0,1,0xb27f7,0x80000000u,0xffffffffu};
    constexpr std::array<std::array<std::uint32_t,4>,5> reference{{
        {577655601,1248161417,192959563,143633695},
        {1804289383,846930886,1681692777,1714636915},
        {134421688,624679831,1175454998,389078347},
        {1336741213,1210407648,1447044896,337392383},
        {254925627,1205188300,366127624,1401405153}}};
    for(std::size_t j=0;j<seeds.size();++j) {
        md::DarwinRandom rng(seeds[j]);
        for(auto n:reference[j]) check(rng.next()==n,"Darwin random fixed reference");
#ifdef __APPLE__
        md::DarwinRandom longRun(seeds[j]); ::srandom(seeds[j]);
        for(int i=0;i<10000;++i)check(longRun.next()==static_cast<std::uint32_t>(::random()),"Darwin libc sequence");
#endif
    }
    std::vector<std::uint8_t> bytes(md::SaveData::byteCount,0);
    for(std::size_t i=4;i<bytes.size()-4;++i)bytes[i]=(i*37+19)&255;
    auto s=md::SaveData::fromPlaintext(bytes);
    s.writeWord(0,0xffffffffu);
    auto encoded=s.encode();auto loaded=md::SaveData::decode(encoded);
    check(loaded.encode()==encoded,"save byte roundtrip");
    for(std::size_t slot=0;slot<5;++slot){loaded.setCredits(slot,123456+static_cast<int>(slot));check(loaded.credits(slot)==123456+static_cast<int>(slot),"slot isolation");}
    rejects([&]{loaded.setCredits(5,1);},"slot boundary");
    rejects([&]{loaded.setCredits(0,-1);},"negative balance");
    rejects([&]{loaded.readWord(md::SaveData::byteCount-3);},"word boundary");
    auto corrupt=encoded;corrupt[100]^=1;
    rejects([&]{md::SaveData::decode(corrupt);},"corrupt save checksum");
    corrupt.resize(0xa5ec); rejects([&]{md::SaveData::decode(corrupt);},"indie format rejection");
    auto profiles=md::SaveData::empty(123);
    for(std::size_t i=0;i<5;++i)check(profiles.createProfile("TEST",1700000000)==i,"first empty slot");
    check(!profiles.createProfile("FULL",1700000000),"five-slot limit");
    profiles.deleteProfile(2);check(profiles.createProfile("REUSED",1700000001)==2,"deleted slot reuse");
    auto created=md::SaveData::decode(profiles.encode());check(created.name(2)=="REUSED","new profile persistence");
    auto legacy=md::SaveData::empty(123);legacy.createProfile("OLD",100);
    legacy.writeWord(md::SaveData::slotStart+0x38,2);legacy.setCredits(0,321);
    legacy.selectProfile(0,200);
    check(legacy.readWord(md::SaveData::slotStart+0x2054)==3821,"original legacy score migration");
    legacy.selectProfile(0,300);
    check(legacy.readWord(md::SaveData::slotStart+0x2054)==3821,"score migration happens once");
    for(int arg=1;arg<argc;++arg) {
        std::ifstream f(argv[arg],std::ios::binary);check(f.good(),"fixture open");
        const std::vector<std::uint8_t> original((std::istreambuf_iterator<char>(f)),{});
        const auto fixture=md::SaveData::decode(original);
        check(fixture.encode()==original,"original WeMade save exact roundtrip");
        check(fixture.active(0)&&fixture.name(0)=="1234","original profile");
        if(fixture.credits(0)==0) {
            auto recreated=md::SaveData::empty(fixture.readWord(0));
            recreated.createProfile("1234",fixture.readWord(md::SaveData::slotStart));
            // New product is English-only: original fixture is KR, expected US change only.
            const auto x=recreated.plaintext(),y=fixture.plaintext();
            for(std::size_t i=md::SaveData::slotStart;i<md::SaveData::slotStart+md::SaveData::slotStride;++i) {
                if(i==md::SaveData::slotStart+0x30 || i==md::SaveData::slotStart+0x31)continue;
                if(x[i]!=y[i]){std::cerr<<"profile mismatch offset "<<std::hex<<(i-md::SaveData::slotStart)<<std::dec<<'\n';check(false,"original new profile exact bytes");}
            }
        }
        std::cout<<"original save: slot0="<<fixture.name(0)<<" credits="<<fixture.credits(0)<<" bytes="<<original.size()<<'\n';
    }
    std::cout<<"PASS: RNG, five slots, byte preservation, corruption and variant rejection\n";
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
