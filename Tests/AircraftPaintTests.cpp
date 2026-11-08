#include "Core/Presentation/AircraftPaint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <vector>
int main(){
    std::vector<std::uint8_t> samples;
    for(unsigned a=0;a<256;++a)for(unsigned v=0;v<256;++v){
        samples.push_back(v);samples.push_back((v+37)%256);
        samples.push_back((v+91)%256);samples.push_back(a);
    }
    for(auto name:{"E_Tank32_Body.png","E_Turret_Body_01.png","P_Body_01.png","Turret_mg_05.png","GameUI_MachSlot.png","unknown.png"}){
        auto pixels=samples;md::prepareAircraftPaint(name,pixels.data(),pixels.size()/4);assert(pixels==samples);
    }
    for(auto name:{"ALightBody.png","AHeavyBody.png","ATransportBody.png","ADuctBody.png"}){
        auto pixels=samples;md::prepareAircraftPaint(name,pixels.data(),pixels.size()/4);assert(pixels!=samples);
        for(std::size_t i=0;i<pixels.size();i+=4){
            assert(pixels[i+3]==samples[i+3]);
            if(!samples[i+3])assert(std::equal(pixels.begin()+i,pixels.begin()+i+4,samples.begin()+i));
        }
        std::array<std::uint8_t,1024> ramp{};
        for(unsigned v=0;v<256;++v){ramp[4*v]=ramp[4*v+1]=ramp[4*v+2]=v;ramp[4*v+3]=255;}
        md::prepareAircraftPaint(name,ramp.data(),256);
        for(unsigned c=0;c<3;++c){assert(ramp[c]==0);assert(ramp[1020+c]==255);for(unsigned v=1;v<256;++v)assert(ramp[4*v+c]>=ramp[4*(v-1)+c]);}
    }
    std::array<std::uint8_t,4> ivory{100,100,90,255},steel{120,110,90,255};
    md::prepareAircraftPaint("ATransportBody.png",ivory.data(),1);assert(ivory[0]>130&&ivory[1]>130&&ivory[2]>110);
    md::prepareAircraftPaint("AHeavyBody.png",steel.data(),1);assert(steel[0]-std::min(steel[0],steel[2])<15);
    auto runtime=samples;md::applyAircraftPaint("ALightBody.png",runtime.data(),runtime.size()/4);assert(runtime==samples);
    std::cout<<"PASS: original units unchanged; aircraft alpha, transparency, endpoints and tone order preserved\n";
}
