#pragma once
#include "Core/Contracts/Game.hpp"
#include <algorithm>
#include <cstdio>
namespace md {
// GuiHelp 0x2a550/0x2a8a8/0x2a950; original English raster pages.
class Help {
public:
    explicit Help(int page=-1):page_(std::clamp(page,0,6)),index_(page<0),navigation_(page<0){}
    void append(Frame& f)const{
        Sprite black;black.destination={0,0,f.logicalWidth,f.logicalHeight};black.color={0,0,0,1};f.sprites.push_back(black);
        char name[64];if(index_)std::snprintf(name,sizeof name,"iPon_HelpScrMain_us@2x.jpg");
        else std::snprintf(name,sizeof name,"iPon_HelpScr%02d_us@2x.jpg",page_+1);
        Sprite page;page.image=name;page.destination={0,0,320,480};f.sprites.push_back(std::move(page));
    }
    // true means the original close region was released.
    bool touch(const Touch& t){
        if(t.phase!=TouchPhase::ended)return false;
        if(!navigation_)return true;
        if(index_){if(t.position.y>360)return true;index_=false;page_=0;return false;}
        if(t.position.x<80)page_=std::max(0,page_-1);
        else if(t.position.x>240)page_=std::min(6,page_+1);
        else if(t.position.y>360)return true;
        else index_=true;
        return false;
    }
private:int page_=0;bool index_=false,navigation_=false;
};
}
