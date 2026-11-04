#include "Transition.hpp"
#include <cmath>

namespace md {
Transition::Transition(ResourceAccess& r,HostServices& h,bool ipad):resources_(r),host_(h),ipad_(ipad){
    if(ipad_){start_={1408,-384,-256,1024};destination_={640,384,256,512};checkX_=200;}
    else{start_={672,-192,-128,448};destination_={304,176,96,224};checkX_=80;}
}
bool Transition::request(unsigned target){
    if(!canChangeScreen()||target<1||target>5)return false;
    target_=target;state_=0;return true;
}
float Transition::approach(float from,float to,float divisor){
    return std::fabs(from-to)>.1f?from+(to-from)/divisor:to;
}
std::optional<unsigned> Transition::tick(){
    std::optional<unsigned> refresh;
    switch(state_){
    case 0:
        visible_=true;
        positions_={Vec2{ipad_?384.f:160.f,start_[0]},Vec2{ipad_?384.f:160.f,start_[1]},
                    Vec2{start_[2],ipad_?512.f:240.f},Vec2{start_[3],ipad_?512.f:240.f}};
        centerScale_=1.5f;state_=1;host_.playSound("Loading_Close.wav");break;
    case 1:
        positions_[2].x+=(destination_[2]-positions_[2].x)/5;
        positions_[3].x+=(destination_[3]-positions_[3].x)/5;
        if(positions_[2].x>=checkX_){state_=2;host_.playSound("Loading_Close.wav");}break;
    case 2:
        positions_[0].y=approach(positions_[0].y,destination_[0],6);
        positions_[1].y=approach(positions_[1].y,destination_[1],6);
        positions_[2].x=approach(positions_[2].x,destination_[2],5);
        positions_[3].x=approach(positions_[3].x,destination_[3],5);
        if(positions_[0].y==destination_[0]){positions_[1].y=destination_[1];state_=3;}break;
    case 3:
        centerScale_+=(.9f-centerScale_)/5;
        if(centerScale_<1){centerScale_=1;state_=4;}break;
    case 4:
        refresh=target_;state_=5;host_.playSound("Loading_Open.wav");break;
    case 5:
        centerScale_+=(1.5f-centerScale_)/5;
        if(centerScale_>1.4f)state_=7;break;
    case 6:
        // Present in the original state machine; normal state 5 goes directly to 7.
        positions_[2].x=approach(positions_[2].x,start_[2],10);
        positions_[3].x=approach(positions_[3].x,start_[3],10);
        if(positions_[2].x<0){state_=7;host_.playSound("Loading_Open.wav");}break;
    case 7:
        centerScale_+=(1.5f-centerScale_)/5;
        positions_[0].y=approach(positions_[0].y,start_[0],10);
        positions_[1].y=approach(positions_[1].y,start_[1],10);
        positions_[2].x=approach(positions_[2].x,start_[2],10);
        positions_[3].x=approach(positions_[3].x,start_[3],10);
        if(positions_[0].y==start_[0]){centerScale_=1.5f;visible_=false;state_=8;}break;
    default:break;
    }
    return refresh;
}
void Transition::append(Frame& frame)const{
    if(!visible_)return;
    const auto draw=[&](std::string image,Vec2 center,float scale){
        const auto px=resources_.imageSize(image);const float ratio=(ipad_?1.f:.5f)*scale;
        Sprite s;s.image=std::move(image);s.sourcePx=Rect{0,0,px.x,px.y};
        s.destination={center.x-px.x*ratio*.5f,frame.logicalHeight-center.y-px.y*ratio*.5f,px.x*ratio,px.y*ratio};
        frame.sprites.push_back(std::move(s));
    };
    draw("Loading_Left.png",positions_[2],2);draw("Loading_Right.png",positions_[3],2);
    draw("Loading_Top.png",positions_[0],1.5f);draw("Loading_Bottom.png",positions_[1],1.5f);
    draw("Loading_Center.png",{positions_[1].x,positions_[1].y+(ipad_?168.f:84.f)},centerScale_);
}
}
