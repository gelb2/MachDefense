// Read-only audit of the production texture-preparation transform.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Core/Presentation/AircraftPaint.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>
int main(int argc,char** argv){
    assert(argc==2);unsigned originals=0,aircraft=0,markedBodies=0;
    std::cout<<"{\"textures\":[";bool first=true;
    for(const auto& f:std::filesystem::directory_iterator(argv[1])){
        if(f.path().extension()!=".png")continue;
        int w,h,c;auto* p=stbi_load(f.path().c_str(),&w,&h,&c,4);assert(p);
        const std::string name=f.path().filename().string();
        const std::size_t count=std::size_t(w)*h;
        std::vector<std::uint8_t> before(p,p+count*4),base=before,paint=before;
        stbi_image_free(p);
        md::applyAircraftPaint(name,base.data(),count);
        md::applyAircraftPaint(name,paint.data(),count,w,h);
        unsigned visible=0,changed=0,marked=0;
        for(std::size_t i=0;i<count;++i){
            assert(before[4*i+3]==paint[4*i+3]);
            if(before[4*i+3])++visible;
            bool delta=false,mark=false;
            for(unsigned k=0;k<3;++k){delta|=before[4*i+k]!=paint[4*i+k];mark|=base[4*i+k]!=paint[4*i+k];}
            changed+=delta;marked+=mark;
            if(!before[4*i+3])for(unsigned k=0;k<4;++k)assert(before[4*i+k]==paint[4*i+k]);
        }
        const bool target=md::aircraftPaintMidpoint(name)!=std::array<unsigned,3>{128,128,128};
        if(!target){assert(before==paint);++originals;continue;}
        ++aircraft;assert(marked<=visible/8+1);
        if(name=="ALightBody.png"||name=="ATransportBody.png"||name=="AHeavyBody.png"||name=="ADuctBody.png"){
            assert(marked>0);++markedBodies;
        }
        if(!first)std::cout<<',';first=false;
        std::cout<<"{\"name\":\""<<name<<"\",\"size\":["<<w<<','<<h<<"],\"visible\":"<<visible<<",\"colourChanged\":"<<changed<<",\"markingPixels\":"<<marked<<",\"alphaUnchanged\":true}";
    }
    assert(markedBodies==4);
    std::cout<<"],\"originalTexturesUnchanged\":"<<originals<<",\"aircraftTextures\":"<<aircraft<<",\"markedBodies\":"<<markedBodies<<",\"allAlphaUnchanged\":true}\n";
}
