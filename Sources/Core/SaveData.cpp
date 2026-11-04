#include "SaveData.hpp"
#include "DarwinRandom.hpp"
#include <algorithm>
#include <bit>
#include <stdexcept>

namespace md {
namespace {
std::uint32_t read32(std::span<const std::uint8_t> b, std::size_t offset) {
    if (offset > b.size() || b.size()-offset < 4) throw std::out_of_range("save word");
    return b[offset] | (std::uint32_t(b[offset+1])<<8) |
        (std::uint32_t(b[offset+2])<<16) | (std::uint32_t(b[offset+3])<<24);
}
std::int32_t checksum(std::span<const std::uint8_t> b) {
    std::int32_t sum=0;
    for (std::size_t i=4; i<b.size()-4; ++i) sum+=b[i]<128 ? b[i] : int(b[i])-256;
    return sum;
}
void crypt(std::span<std::uint8_t> b) {
    DarwinRandom rng(read32(b,0));
    for (std::size_t i=4; i<b.size()-4; ++i) b[i]^=rng.next()&255;
}
}
SaveData SaveData::fromPlaintext(std::span<const std::uint8_t> bytes) {
    if (bytes.size()!=byteCount) throw std::invalid_argument("WeMade save must contain 42496 bytes");
    SaveData result; std::copy(bytes.begin(),bytes.end(),result.bytes_.begin()); return result;
}
SaveData SaveData::empty(std::uint32_t seed) {
    SaveData result;result.writeWord(0,seed);return result;
}
void SaveData::putString(std::size_t offset,std::size_t capacity,std::string_view value) {
    if(offset>byteCount || capacity>byteCount-offset || value.size()>=capacity || value.find('\0')!=value.npos)
        throw std::invalid_argument("save string capacity");
    std::fill_n(bytes_.begin()+offset,capacity,0);
    std::copy(value.begin(),value.end(),bytes_.begin()+offset);
}
std::optional<std::size_t> SaveData::createProfile(std::string_view name,std::uint32_t timestamp) {
    if(name.empty() || name.size()>31 || name.find('\0')!=name.npos)
        throw std::invalid_argument("profile name must contain 1..31 bytes");
    for(std::size_t i=0;i<slotCount;++i) {
        if(active(i))continue;
        const auto at=slotOffset(i);
        std::fill_n(bytes_.begin()+at,slotStride,0);
        writeWord(at,timestamp);writeWord(at+4,timestamp);bytes_[at+8]=1;
        putString(at+0x10,32,name);putString(at+0x30,4,"us");
        // GlobalInfo::initSlotWithName:Country: and selectSlot: initial equipment.
        // buyBuildSet: 0x1d400 inserts by type/cost; this is the resulting default order.
        constexpr std::array<std::string_view,4> machines{"MachineGun","Missile","Tanker","RifleTurret"};
        writeWord(at+0xb44,machines.size());
        for(std::size_t j=0;j<machines.size();++j){const auto e=at+0xb48+j*0x28;bytes_[e]=1;putString(e+1,32,machines[j]);}
        constexpr std::array<std::string_view,3> attacks{"AirStrike-Bomb","AirStrike-Missile","AirStrike-Nuclear"};
        writeWord(at+0x40,attacks.size());
        for(std::size_t j=0;j<attacks.size();++j){const auto e=at+0x44+j*0x2c;bytes_[e]=1;putString(e+1,32,attacks[j]);writeWord(e+0x28,j==2?1:2);}
        writeWord(at+0x2058,0x3f4ccccd);writeWord(at+0x205c,0x3f800000);
        return i;
    }
    return std::nullopt;
}
void SaveData::deleteProfile(std::size_t slot) {
    const auto at=slotOffset(slot);std::fill_n(bytes_.begin()+at,slotStride,0);
}
void SaveData::selectProfile(std::size_t slot,std::uint32_t timestamp) {
    if(!active(slot))throw std::invalid_argument("empty profile");
    const auto at=slotOffset(slot);
    const auto completed=stage(slot);
    if(completed>80)throw std::invalid_argument("profile stage outside original campaign");
    writeWord(slotOffset(slot)+4,timestamp);
    // GlobalInfo::selectSlot: 0x1a7c0 migrates older progress whose score
    // field is zero: 1000, 2500, 4000, ... plus the remaining credits.
    if(completed>0&&readWord(at+0x2054)==0){
        std::uint32_t total=0,reward=1000;
        for(std::uint32_t i=0;i<completed;++i){total+=reward;reward+=1500;}
        writeWord(at+0x2054,total+readWord(at+0x3c));
    }
}
std::uint32_t SaveData::stage(std::size_t slot) const {return readWord(slotOffset(slot)+0x38);}
unsigned SaveData::baseUpgrade(std::size_t slot,unsigned category) const {
    if(category>=6)throw std::out_of_range("base category");
    return bytes_[slotOffset(slot)+0x1f48+category];
}
void SaveData::setBaseUpgrade(std::size_t slot,unsigned category,unsigned level) {
    if(category>=6 || level>255)throw std::out_of_range("base upgrade");
    bytes_[slotOffset(slot)+0x1f48+category]=static_cast<std::uint8_t>(level);
}
std::vector<OwnedBuild> SaveData::builds(std::size_t slot)const {
    const auto at=slotOffset(slot);const auto count=readWord(at+0xb44);
    if(count>128)throw std::invalid_argument("build inventory count");
    std::vector<OwnedBuild> result;
    for(std::size_t i=0;i<count;++i){const auto e=at+0xb48+i*0x28;
        const auto begin=bytes_.begin()+e+1,end=std::find(begin,begin+32,0);
        if(end==begin+32)throw std::invalid_argument("unterminated build name");
        result.push_back({std::string(begin,end),readWord(e+0x24),(bytes_[e]&1)!=0});
    }return result;
}
std::vector<OwnedAttack> SaveData::attacks(std::size_t slot)const {
    const auto at=slotOffset(slot);const auto count=readWord(at+0x40);
    if(count>64)throw std::invalid_argument("special attack inventory count");
    std::vector<OwnedAttack> result;
    for(std::size_t i=0;i<count;++i){const auto e=at+0x44+i*0x2c;
        const auto begin=bytes_.begin()+e+1,end=std::find(begin,begin+32,0);
        if(end==begin+32)throw std::invalid_argument("unterminated attack name");
        result.push_back({std::string(begin,end),readWord(e+0x24),readWord(e+0x28),(bytes_[e]&1)!=0});
    }return result;
}
void SaveData::setBuilds(std::size_t slot,std::span<const OwnedBuild> values){
    if(values.size()>128)throw std::invalid_argument("build inventory capacity");
    for(const auto& value:values)if(value.name.size()>31||value.name.find('\0')!=value.name.npos)throw std::invalid_argument("build name capacity");
    const auto at=slotOffset(slot);writeWord(at+0xb44,std::uint32_t(values.size()));
    for(std::size_t i=0;i<values.size();++i){const auto e=at+0xb48+i*0x28;bytes_[e]=values[i].onSlot;putString(e+1,32,values[i].name);writeWord(e+0x24,values[i].level);}
}
void SaveData::setAttacks(std::size_t slot,std::span<const OwnedAttack> values){
    if(values.size()>64)throw std::invalid_argument("attack inventory capacity");
    for(const auto& value:values)if(value.name.size()>31||value.name.find('\0')!=value.name.npos)throw std::invalid_argument("attack name capacity");
    const auto at=slotOffset(slot);writeWord(at+0x40,std::uint32_t(values.size()));
    for(std::size_t i=0;i<values.size();++i){const auto e=at+0x44+i*0x2c;bytes_[e]=values[i].onSlot;putString(e+1,32,values[i].name);writeWord(e+0x24,values[i].level);writeWord(e+0x28,values[i].count);}
}
int SaveData::clearCount(std::size_t slot,unsigned mission)const {
    if(mission>=80)throw std::out_of_range("mission clear count");return std::bit_cast<std::int8_t>(bytes_[slotOffset(slot)+0x1f58+mission]);
}
void SaveData::setClearCount(std::size_t slot,unsigned mission,unsigned count){
    if(mission>=80)throw std::out_of_range("mission clear count");bytes_[slotOffset(slot)+0x1f58+mission]=std::uint8_t(count);
}
SaveData SaveData::decode(std::span<const std::uint8_t> encoded) {
    SaveData result=fromPlaintext(encoded);
    crypt(result.bytes_);
    if (std::bit_cast<std::int32_t>(result.readWord(byteCount-4))!=checksum(result.bytes_))
        throw std::invalid_argument("WeMade save checksum mismatch");
    return result;
}
std::vector<std::uint8_t> SaveData::encode() const {
    auto copy=*this;
    copy.writeWord(byteCount-4,std::bit_cast<std::uint32_t>(checksum(bytes_)));
    crypt(copy.bytes_);
    return {copy.bytes_.begin(),copy.bytes_.end()};
}
std::size_t SaveData::slotOffset(std::size_t slot) {
    if (slot>=slotCount) throw std::out_of_range("save slot");
    return slotStart+slot*slotStride;
}
bool SaveData::active(std::size_t slot) const { return bytes_[slotOffset(slot)+8]!=0; }
std::string SaveData::name(std::size_t slot) const {
    const auto at=slotOffset(slot)+0x10;
    auto end=std::find(bytes_.begin()+at,bytes_.begin()+at+32,0);
    return {bytes_.begin()+at,end};
}
std::int32_t SaveData::credits(std::size_t slot) const {
    return std::bit_cast<std::int32_t>(readWord(slotOffset(slot)+0x3c));
}
void SaveData::setCredits(std::size_t slot, std::int32_t value) {
    if (value<0) throw std::invalid_argument("negative credits");
    writeWord(slotOffset(slot)+0x3c,std::bit_cast<std::uint32_t>(value));
}
std::uint32_t SaveData::readWord(std::size_t offset) const { return read32(bytes_,offset); }
void SaveData::writeWord(std::size_t offset,std::uint32_t value) {
    if (offset>byteCount || byteCount-offset<4) throw std::out_of_range("save word");
    for(unsigned i=0;i<4;++i) bytes_[offset+i]=(value>>(i*8))&255;
}
}
