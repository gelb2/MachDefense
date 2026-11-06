#pragma once
#include "Core/Session/SaveData.hpp"
#include <algorithm>
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>

namespace md::transfer {
using Bytes = std::vector<std::uint8_t>;
// Used only when restoring a profile file or opening the app. Normal game
// saves retain their existing host path. All native hosts use this protocol.
class FileStore {
    std::string path_;
    bool syncDirectory() const {
        const auto slash=path_.find_last_of('/');
        const std::string dir=slash==std::string::npos?".":path_.substr(0,slash);
        const int fd=::open(dir.c_str(),O_RDONLY);if(fd<0)return false;
        const bool ok=::fsync(fd)==0;::close(fd);return ok;
    }
    static std::optional<Bytes> read(const std::string& p){
        const int fd=::open(p.c_str(),O_RDONLY);
        if(fd<0){if(errno==ENOENT)return {};throw std::runtime_error("Cannot read recovery file");}
        struct stat st{};if(::fstat(fd,&st)||st.st_size<0||st.st_size>SaveData::byteCount){::close(fd);throw std::runtime_error("Invalid recovery file size");}
        Bytes data(static_cast<size_t>(st.st_size));size_t at=0;
        while(at<data.size()){const auto n=::read(fd,data.data()+at,data.size()-at);if(n<0&&errno==EINTR)continue;if(n<=0){::close(fd);throw std::runtime_error("Cannot read recovery file");}at+=size_t(n);}
        ::close(fd);return data;
    }
    bool atomic(const std::string& p,std::span<const std::uint8_t> b){
        const auto tmp=p+".tmp";const int fd=::open(tmp.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd<0)return false;
        size_t at=0;bool ok=true;while(at<b.size()){auto n=::write(fd,b.data()+at,b.size()-at);if(n<0&&errno==EINTR)continue;if(n<=0){ok=false;break;}at+=size_t(n);}
        if(ok)ok=::fsync(fd)==0;if(::close(fd))ok=false;
        if(ok)ok=::rename(tmp.c_str(),p.c_str())==0;
        if(!ok){::unlink(tmp.c_str());return false;}return syncDirectory();
    }
    bool erase(const std::string& p){if(::unlink(p.c_str())&&errno!=ENOENT)return false;return syncDirectory();}
    static void valid(std::span<const std::uint8_t> b){(void)SaveData::decode(b);}
public:
    enum class Stage { backedUp, prepared, replaced };
    // Fault-injection hook is unused by product hosts.
    std::function<void(Stage)> checkpoint;
    explicit FileStore(std::string path):path_(std::move(path)){}
    bool recover(){
        const auto pending=read(path_+".restore-pending");
        const auto committed=read(path_+".restore-committed");
        if(!pending)return !committed||erase(path_+".restore-committed");
        valid(*pending);
        if(committed&&*committed==*pending){
            // The game may already have saved newer progress after this commit.
            // A leftover journal must never overwrite that progress.
            if(!erase(path_+".restore-pending"))return false;
            return erase(path_+".restore-committed");
        }
        const auto current=read(path_);
        if(!current||*current!=*pending){if(!atomic(path_,*pending))return false;}
        if(!syncDirectory())return false;
        const auto verified=read(path_);if(!verified||*verified!=*pending)return false;
        if(!atomic(path_+".restore-committed",*pending))return false;
        if(!erase(path_+".restore-pending"))return false;
        return erase(path_+".restore-committed");
    }
    bool restore(std::span<const std::uint8_t> b){
        valid(b);if(!recover())return false;
        const auto before=read(path_);if(before)valid(*before);
        // Empty backup records the absence of an original file.
        if(!atomic(path_+".restore-backup",before?std::span<const uint8_t>(*before):std::span<const uint8_t>{}))return false;
        if(checkpoint)checkpoint(Stage::backedUp);
        if(!atomic(path_+".restore-pending",b)){
            if(!erase(path_+".restore-pending"))throw std::runtime_error("Recovery preparation could not be cleared");return false;
        }
        if(checkpoint)checkpoint(Stage::prepared);
        bool installed=atomic(path_,b);
        if(checkpoint)checkpoint(Stage::replaced);
        const auto after=read(path_);installed=installed&&after&&std::equal(after->begin(),after->end(),b.begin(),b.end());
        if(installed&&atomic(path_+".restore-committed",b)){
            // The durable commit marker protects subsequent gameplay saves even
            // when removal of the completed journal needs a startup retry.
            if(erase(path_+".restore-pending"))(void)erase(path_+".restore-committed");
            return true;
        }
        if(!erase(path_+".restore-committed"))throw std::runtime_error("Recovery commit could not be cleared");
        const bool rolledBack=before?atomic(path_,*before):erase(path_);
        if(!rolledBack)throw std::runtime_error("Recovery interrupted; reopen the app to finish");
        if(!erase(path_+".restore-pending"))throw std::runtime_error("Recovery journal could not be cleared");
        return false;
    }
};
}
