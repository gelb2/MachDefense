#pragma once
#include "Assets.hpp"
#include <GLES/gl.h>
#include <map>
namespace md::android {
struct Texture {GLuint id=0;int width=0,height=0,storageWidth=0,storageHeight=0;};
struct RenderMetrics{unsigned drawCalls=0,textureBinds=0,filterChanges=0,blendChanges=0,clipChanges=0,sprites=0;};
class Renderer {
    Assets& assets_;std::map<std::string,Texture> _textures;
    int _width=0,_height=0;float _logicalWidth=0,_logicalHeight=0;Rect viewport_;
    Texture* texture(const std::string&,bool);
public:
    explicit Renderer(Assets& assets):assets_(assets){}
    unsigned textureUploads=0;RenderMetrics _metrics;
    void lostContext(){_textures.clear();}
    void release();
    bool draw(const Frame&,int width,int height);
    std::optional<Vec2> logicalPoint(float,float)const;
};
}
