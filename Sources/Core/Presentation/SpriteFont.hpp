#pragma once
#include "Core/Data/DataTable.hpp"
#include "Core/Contracts/RenderTypes.hpp"
#include <array>
#include <stdexcept>
#include <string_view>

namespace md {
// GuiSelectSlot/GuiSelectStage use TrebuchetMS-Bold (original CFString 0x6aaac).
// Glyphs are rasterized once by tools/prepare_font.swift, never during play.
class SpriteFont {
public:
    explicit SpriteFont(std::string_view metrics) {
        const auto table=DataTable::parse(metrics);
        if(table.rowCount()!=95)throw std::runtime_error("Incomplete ASCII glyph atlas");
        for(std::size_t i=0;i<table.rowCount();++i){
            const auto code=table.integer(i,"Code");const auto advance=table.number(i,"Advance");
            if(!code||*code<32||*code>126||!advance)throw std::runtime_error("Invalid glyph metric");
            advances_[std::size_t(*code-32)]=*advance;
        }
    }
    float width(std::string_view text,float size) const {
        float w=0;for(unsigned char c:text)w+=advances_[index(c)]*size/48;return w;
    }
    void draw(Frame& frame,std::string_view text,Vec2 topLeft,float size,Color color={}) const {
        const float s=size/48;
        for(unsigned char c:text){
            const auto i=index(c);
            if(c!=' '){
                Sprite glyph;glyph.image="Generated/TrebuchetMS-Bold-48.png";
                glyph.sourcePx=Rect{float(i%16)*80,float(i/16)*80,80,80};
                glyph.destination={topLeft.x-8*s,topLeft.y-8*s,80*s,80*s};glyph.color=color;
                frame.sprites.push_back(std::move(glyph));
            }
            topLeft.x+=advances_[i]*s;
        }
    }
private:
    static std::size_t index(unsigned char c){return (c>=32&&c<=126?c:'?')-32;}
    std::array<float,95> advances_{};
};
}
