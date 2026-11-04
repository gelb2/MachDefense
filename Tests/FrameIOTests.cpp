#include "SessionTestSupport.hpp"
#include "ValidationTools.hpp"
struct CountingResources:Resources {
    std::map<std::string,unsigned> reads;
    Bytes read(std::string_view path)override {++reads[std::string(path)];return Resources::read(path);}
    unsigned dataReads()const {unsigned n=0;for(const auto& [path,count]:reads)if(path.starts_with("Tables/")||path.starts_with("Generated/"))n+=count;return n;}
};
struct AuditHost:Host {bool fail=false;bool writeSave(std::span<const std::uint8_t> b)override {if(fail)return false;return Host::writeSave(b);}};
int main(int argc,char** argv){assert(argc==2);CountingResources r;r.root=argv[1];AuditHost h;
    auto save=SaveData::empty(1);save.createProfile("IO TEST",1);save.writeWord(SaveData::slotStart+0xc,5);save.setClearCount(0,0,1);h.saved=save.encode();
    GameSession s(r,h,true);s.applyValidationCommand(unsigned(ValidationCommand::startMission));ticks(s);auto* world=s.validationWorld();assert(world&&!world->paused());
    world->setValidationCells(10000);world->enqueueBuild("MachineGun");
    auto before=r.dataReads();const auto writes=h.writes;ticks(s,1200);
    assert(world->state()==WorldState::running&&world->statistics().produced>0);assert(h.writes==writes);
    bool failed=false;if(r.dataReads()!=before){std::cerr<<"combat frame data reads: "<<r.dataReads()-before<<'\n';failed=true;}
    h.fail=true;s.applyValidationCommand(unsigned(ValidationCommand::save));assert(has(s.frame(),"MessageBox.png"));before=r.dataReads();ticks(s,60);
    if(r.dataReads()!=before){std::cerr<<"error dialog frame data reads: "<<r.dataReads()-before<<'\n';failed=true;}
    tap(s,160,295);h.fail=false;s.applyValidationCommand(unsigned(ValidationCommand::win));assert(world->state()==WorldState::cleared);assert(h.writes==writes+1);
    before=r.dataReads();const auto resultWrites=h.writes;ticks(s,60);
    if(r.dataReads()!=before){std::cerr<<"reward dialog frame data reads: "<<r.dataReads()-before<<'\n';failed=true;}
    assert(h.writes==resultWrites);if(failed)return 1;
    std::cout<<"Combat 1200 ticks: no save writes; combat/error/reward frames: no table or font file reads\n";
}
