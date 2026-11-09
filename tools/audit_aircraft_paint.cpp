// Read-only audit of the production texture-preparation transform.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Core/Presentation/AircraftPaint.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>
int main(int argc,char** argv){
    stbi_convert_iphone_png_to_rgb(1);stbi_set_unpremultiply_on_load(1);
    assert(argc==2||argc==3);unsigned originals=0,aircraft=0,markedBodies=0;
    std::cout<<"{\"textures\":[";bool first=true;
    for(const auto& f:std::filesystem::directory_iterator(argv[1])){
        if(f.path().extension()!=".png")continue;
        int w,h,c;auto* p=stbi_load(f.path().c_str(),&w,&h,&c,4);assert(p);
        const std::string name=f.path().filename().string();
        const std::size_t count=std::size_t(w)*h;
        std::vector<std::uint8_t> before(p,p+count*4),base=before,paint=before;
        stbi_image_free(p);
        md::prepareAircraftPaint(name,base.data(),count);
        md::prepareAircraftPaint(name,paint.data(),count,w,h);
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
        if(argc==3){
            int ow,oh,oc;auto output=std::filesystem::path(argv[2])/name;
            auto* q=stbi_load(output.c_str(),&ow,&oh,&oc,4);assert(q&&(ow==w||ow==w*7)&&oh==h);
            // Apple CgBI stores premultiplied integer channels. Keep exact alpha
            // and opaque pixels; partial-alpha rounding stays below one output
            // channel level after compositing, verified by the decode audit.
            for(std::size_t i=0;i<count;++i){
                const auto j=(i/std::size_t(w)*ow+i%w)*4;
                const auto a=paint[i*4+3];assert(q[j+3]==a);
                for(unsigned c=0;c<3;++c){
                    const auto difference=std::abs(int(paint[i*4+c])-int(q[j+c]));
                    if(a==255)assert(difference==0);
                    assert(difference*unsigned(a)<=255);
                }
            }
            std::vector<std::uint8_t> decoded(q,q+std::size_t(ow)*oh*4);
            md::applyAircraftPaint(name,q,std::size_t(ow)*oh,ow,oh);
            assert(std::equal(decoded.begin(),decoded.end(),q));stbi_image_free(q);
        }
        if(name=="ALightBody.png"||name=="ATransportBody.png"||name=="AHeavyBody.png"||name=="ADuctBody.png"){
            assert(marked>0);++markedBodies;
        }
        if(!first)std::cout<<',';first=false;
        std::cout<<"{\"name\":\""<<name<<"\",\"size\":["<<w<<','<<h<<"],\"visible\":"<<visible<<",\"colourChanged\":"<<changed<<",\"markingPixels\":"<<marked<<",\"alphaUnchanged\":true}";
    }
    assert(markedBodies==4);
    std::cout<<"],\"originalTexturesUnchanged\":"<<originals<<",\"aircraftTextures\":"<<aircraft<<",\"markedBodies\":"<<markedBodies<<",\"allAlphaUnchanged\":true}\n";
}
