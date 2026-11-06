#include "../Modules/SaveTransfer/FileStore.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace md;
using Bytes = std::vector<std::uint8_t>;
static void put(const std::string& path,const Bytes& b){std::ofstream f(path,std::ios::binary);f.write((const char*)b.data(),b.size());}
static Bytes get(const std::string& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
 auto root=std::filesystem::temp_directory_path()/("md-transfer-"+std::to_string(getpid()));std::filesystem::create_directories(root);auto p=(root/"MachDefense.sav").string();
 auto a=SaveData::empty(1);a.createProfile("OLD",10);auto b=a;b.setCredits(0,1234);auto c=b;c.setCredits(0,5678);const auto old=a.encode(),next=b.encode(),newer=c.encode();
 transfer::FileStore store(p);assert(store.recover());assert(store.restore(next));assert(get(p)==next);assert(get(p+".restore-backup").empty());
 put(p,old);assert(store.restore(next));assert(get(p+".restore-backup")==old);assert(get(p)==next);
 // Interrupted preparation or replacement must finish from the durable journal.
 for(auto stop:{transfer::FileStore::Stage::backedUp,transfer::FileStore::Stage::prepared,transfer::FileStore::Stage::replaced}){
  put(p,old);store.checkpoint=[&](auto at){if(at==stop)throw std::runtime_error("simulated power loss");};
  try{store.restore(next);assert(false);}catch(const std::runtime_error&){}
  store.checkpoint={};assert(store.recover());assert(get(p)==(stop==transfer::FileStore::Stage::backedUp?old:next));assert(get(p+".restore-backup")==old);
 }
 // A journal left after commit must not undo later mission progress.
 put(p,newer);put(p+".restore-pending",next);put(p+".restore-committed",next);assert(store.recover());assert(get(p)==newer);assert(!std::filesystem::exists(p+".restore-pending"));
 auto bad=next;bad[10]^=0xff;try{store.restore(bad);assert(false);}catch(const std::exception&){}assert(get(p)==newer);
 // An unwritable destination leaves the original bytes untouched.
 std::filesystem::create_directory(p+".restore-backup.tmp");assert(!store.restore(next));assert(get(p)==newer);std::filesystem::remove_all(root);
 std::cout<<"FileStore: exact backup, restart recovery at 3 interruption points, newer progress, invalid/failing writes passed\n";
}
