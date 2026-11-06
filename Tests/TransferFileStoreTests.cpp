#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
// Interpose only this test translation unit. Product binaries use the real
// POSIX calls and do not include fault-injection branches.
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
namespace fault {
 std::string renameTarget,unlinkTarget;
 unsigned renames=0,failRename=0,failSyncAfterRename=0;
 bool directorySyncPending=false;
 void reset(){renameTarget.clear();unlinkTarget.clear();renames=failRename=failSyncAfterRename=0;directorySyncPending=false;}
}
static int testRename(const char* from,const char* to){
 const bool target=fault::renameTarget==to;
 if(target&&++fault::renames==fault::failRename){errno=EIO;return -1;}
 const int result=::rename(from,to);
 if(result==0&&target&&fault::renames==fault::failSyncAfterRename)fault::directorySyncPending=true;
 return result;
}
static int testFsync(int fd){
 struct stat info{};
 if(fault::directorySyncPending&&::fstat(fd,&info)==0&&S_ISDIR(info.st_mode)){
  fault::directorySyncPending=false;errno=EIO;return -1;
 }
 return ::fsync(fd);
}
static int testUnlink(const char* path){if(fault::unlinkTarget==path){errno=EACCES;return -1;}return ::unlink(path);}
#define rename testRename
#define fsync testFsync
#define unlink testUnlink
#include "../Modules/SaveTransfer/FileStore.hpp"
#undef rename
#undef fsync
#undef unlink

using namespace md;
using Bytes = std::vector<std::uint8_t>;
static void put(const std::string& path,const Bytes& b){std::ofstream f(path,std::ios::binary);f.write((const char*)b.data(),b.size());}
static Bytes get(const std::string& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char** argv){
 auto root=std::filesystem::temp_directory_path()/("md-transfer-"+std::to_string(getpid()));std::filesystem::create_directories(root);auto p=(root/"MachDefense.sav").string();
 auto a=SaveData::empty(1);a.createProfile("OLD",10);if(argc==2){a=SaveData::decode(get(argv[1]));assert(a.active(0));}auto b=a;b.setCredits(0,1234);auto c=b;c.setCredits(0,5678);const auto old=a.encode(),next=b.encode(),newer=c.encode();
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
 std::filesystem::create_directory(p+".restore-backup.tmp");assert(!store.restore(next));assert(get(p)==newer);std::filesystem::remove(p+".restore-backup.tmp");
 // rename succeeds, but persisting its directory entry fails: restore the old
 // file, report failure and leave no transaction for startup to replay.
 for(const auto& target:{p,p+".restore-pending",p+".restore-committed"}){
  put(p,old);fault::reset();fault::renameTarget=target;fault::failSyncAfterRename=1;
  assert(!store.restore(next));assert(get(p)==old);assert(get(p+".restore-backup")==old);
  assert(!std::filesystem::exists(p+".restore-pending"));fault::reset();assert(store.recover());assert(get(p)==old);
 }
 // If rollback itself fails, do not claim a safe failure. Keep the journal;
 // startup finishes the new save and keeps the old one in the backup.
 put(p,old);fault::reset();fault::renameTarget=p;fault::failSyncAfterRename=1;fault::failRename=2;
 bool interrupted=false;try{store.restore(next);}catch(const std::runtime_error&){interrupted=true;}
 assert(interrupted);assert(get(p+".restore-backup")==old);assert(get(p+".restore-pending")==next);
 fault::reset();assert(transfer::FileStore(p).recover());assert(get(p)==next);
 // Exercise a real cleanup failure after commit, then a later gameplay save.
 put(p,old);fault::unlinkTarget=p+".restore-pending";assert(store.restore(next));
 assert(get(p)==next&&get(p+".restore-committed")==next);put(p,newer);
 fault::reset();assert(transfer::FileStore(p).recover());assert(get(p)==newer);
 // An empty installation must remain empty if its first restore rolls back.
 std::filesystem::remove(p);fault::renameTarget=p;fault::failSyncAfterRename=1;
 assert(!store.restore(next));assert(!std::filesystem::exists(p));fault::reset();assert(store.recover());assert(!std::filesystem::exists(p));
 std::filesystem::remove_all(root);
 std::cout<<"FileStore: exact backup, restart recovery at 3 interruption points, newer progress, invalid writes, 3 rename/fsync failures, rollback failure, cleanup failure and empty-save rollback passed\n";
}
