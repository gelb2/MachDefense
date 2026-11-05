#pragma once
#include "Assets.hpp"
#include <GLES3/gl3.h>
namespace md::web {
class Renderer {
 struct Texture {GLuint id=0;int w=0,h=0,sw=0,sh=0;};
 Assets& assets;std::map<std::string,Texture> textures;GLuint program=0,vbo=0,vao=0,white=0;GLint scale=0;float logicalW=320,logicalH=480;int pixelW=320,pixelH=480;
 Texture& texture(const std::string&,bool);
 void submit(std::span<const MeshVertex>,const Texture&,Blend,bool,std::optional<Rect>);
public:
 explicit Renderer(Assets& a):assets(a){}
 void init();void lost();void draw(const Frame&,int,int);
 unsigned uploads=0,draws=0;
};
}
