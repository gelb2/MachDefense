#include "Renderer.hpp"
#include "Core/Presentation/AircraftPaint.hpp"
#include "stb_image.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace md::web {
namespace {
GLuint shader(GLenum type,const char* text){auto s=glCreateShader(type);glShaderSource(s,1,&text,nullptr);glCompileShader(s);GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[2048];glGetShaderInfoLog(s,sizeof(log),nullptr,log);throw std::runtime_error(log);}return s;}
}
void Renderer::init(){
 const char* vs="#version 300 es\nlayout(location=0) in vec2 p;layout(location=1) in vec2 uv;layout(location=2) in vec4 color;uniform vec2 extent;out vec2 t;out vec4 c;void main(){gl_Position=vec4(p.x/extent.x*2.-1.,1.-p.y/extent.y*2.,0,1);t=uv;c=color;}";
 const char* fs="#version 300 es\nprecision mediump float;in vec2 t;in vec4 c;uniform sampler2D image;out vec4 result;void main(){result=texture(image,t)*c;}";
 auto v=shader(GL_VERTEX_SHADER,vs),f=shader(GL_FRAGMENT_SHADER,fs);program=glCreateProgram();glAttachShader(program,v);glAttachShader(program,f);glLinkProgram(program);glDeleteShader(v);glDeleteShader(f);
 GLint ok;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok)throw std::runtime_error("WebGL shader link failed");scale=glGetUniformLocation(program,"extent");
 glGenVertexArrays(1,&vao);glBindVertexArray(vao);glGenBuffers(1,&vbo);glBindBuffer(GL_ARRAY_BUFFER,vbo);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(MeshVertex),(void*)offsetof(MeshVertex,position));
 glEnableVertexAttribArray(1);glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(MeshVertex),(void*)offsetof(MeshVertex,uv));
 glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(MeshVertex),(void*)offsetof(MeshVertex,color));
 glGenTextures(1,&white);glBindTexture(GL_TEXTURE_2D,white);const unsigned char pixel[]={255,255,255,255};glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
}
void Renderer::lost(){textures.clear();program=vbo=vao=white=0;}
Renderer::Texture& Renderer::texture(const std::string& name,bool premult){
 const auto key=name+(premult?"#p":"#a");if(textures.contains(key))return textures.at(key);
 auto b=assets.read(Assets::imagePath(name));Texture t;int c=0;
 stbi_convert_iphone_png_to_rgb(1);stbi_set_unpremultiply_on_load(1);
 auto* p=stbi_load_from_memory(b.data(),b.size(),&t.w,&t.h,&c,4);if(!p)throw std::runtime_error("Cannot decode "+name);
 applyAircraftPaint(name,p,size_t(t.w)*t.h,t.w,t.h);
 t.sw=t.sh=1;while(t.sw<t.w)t.sw*=2;while(t.sh<t.h)t.sh*=2;
 std::vector<unsigned char> padded(size_t(t.sw)*t.sh*4,0);
 for(int y=0;y<t.h;++y)for(int x=0;x<t.w;++x){auto* src=p+(y*t.w+x)*4;auto* dst=padded.data()+(y*t.sw+x)*4;for(int k=0;k<4;++k)dst[k]=k<3&&premult?static_cast<unsigned char>(float(src[k])*(float(src[3])/255.f)):src[k];}
 stbi_image_free(p);glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t.sw,t.sh,0,GL_RGBA,GL_UNSIGNED_BYTE,padded.data());
 if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("Texture upload: "+name);++uploads;return textures.emplace(key,t).first->second;
}
void Renderer::submit(std::span<const MeshVertex> verts,const Texture& t,Blend blend,bool nearest,std::optional<Rect> clip){
 if(verts.size()<3)return;glBindTexture(GL_TEXTURE_2D,t.id);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,nearest?GL_NEAREST:GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,nearest?GL_NEAREST:GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 if(blend==Blend::opaque)glDisable(GL_BLEND);else{glEnable(GL_BLEND);glBlendFunc(blend==Blend::premultipliedAlpha?GL_ONE:GL_SRC_ALPHA,blend==Blend::additive?GL_ONE:GL_ONE_MINUS_SRC_ALPHA);}
 if(clip){auto c=*clip;float l=std::clamp(c.x,0.f,logicalW),r=std::clamp(c.x+std::max(0.f,c.width),0.f,logicalW),top=std::clamp(c.y,0.f,logicalH),bottom=std::clamp(c.y+std::max(0.f,c.height),0.f,logicalH);glEnable(GL_SCISSOR_TEST);glScissor(l*pixelW/logicalW,(logicalH-bottom)*pixelH/logicalH,std::max(0.f,r-l)*pixelW/logicalW,std::max(0.f,bottom-top)*pixelH/logicalH);}else glDisable(GL_SCISSOR_TEST);
 glBufferData(GL_ARRAY_BUFFER,verts.size_bytes(),verts.data(),GL_STREAM_DRAW);glDrawArrays(GL_TRIANGLE_STRIP,0,verts.size());++draws;
}
void Renderer::draw(const Frame& f,int w,int h){
 pixelW=w;pixelH=h;logicalW=f.logicalWidth;logicalH=f.logicalHeight;draws=0;
 glViewport(0,0,w,h);glDisable(GL_SCISSOR_TEST);glClearColor(f.background.r,f.background.g,f.background.b,f.background.a);glClear(GL_COLOR_BUFFER_BIT);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glUseProgram(program);glUniform2f(scale,logicalW,logicalH);glActiveTexture(GL_TEXTURE0);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
 std::vector<const TextureMesh*> meshes;for(auto& m:f.meshes)meshes.push_back(&m);std::stable_sort(meshes.begin(),meshes.end(),[](auto a,auto b){return a->beforeSpriteIndex<b->beforeSpriteIndex;});size_t mi=0;
 auto mesh=[&](const TextureMesh& m){bool pre=m.blend==Blend::premultipliedAlpha;auto& t=texture(m.image,pre);auto v=m.vertices;for(auto& x:v){x.uv.x*=float(t.w)/t.sw;x.uv.y*=float(t.h)/t.sh;if(pre){x.color.r*=x.color.a;x.color.g*=x.color.a;x.color.b*=x.color.a;}}submit(v,t,m.blend,m.nearest,{});};
 for(size_t i=0;i<=f.sprites.size();++i){while(mi<meshes.size()&&meshes[mi]->beforeSpriteIndex<=i)mesh(*meshes[mi++]);if(i==f.sprites.size())break;
 auto& s=f.sprites[i];auto d=s.destination;if(!std::isfinite(d.x)||!std::isfinite(d.y)||!std::isfinite(d.width)||!std::isfinite(d.height)||d.width==0||d.height==0)continue;
 bool pre=s.blend==Blend::premultipliedAlpha;Texture solid{white,1,1,1,1};const auto& t=s.image.empty()?solid:texture(s.image,pre);auto r=s.sourcePx.value_or(Rect{0,0,float(t.w),float(t.h)});
 float l=r.x/t.sw,rr=(r.x+r.width)/t.sw,top=r.y/t.sh,b=(r.y+r.height)/t.sh;if(s.flipX)std::swap(l,rr);if(s.flipY)std::swap(top,b);
 auto c=s.color;if(pre){c.r*=c.a;c.g*=c.a;c.b*=c.a;}MeshVertex v[]={{{0,0},{l,top},c},{{d.width,0},{rr,top},c},{{0,d.height},{l,b},c},{{d.width,d.height},{rr,b},c}};
 float px=d.width*s.pivot.x,py=d.height*s.pivot.y,rad=s.rotationDegrees*3.14159265358979323846f/180.f,cs=std::cos(rad),sn=std::sin(rad);
 for(auto& x:v){float xx=x.position.x-px,yy=x.position.y-py;x.position={d.x+px+xx*cs-yy*sn,d.y+py+xx*sn+yy*cs};}submit(v,t,s.blend,s.nearest,s.clip);
 }
 while(mi<meshes.size())mesh(*meshes[mi++]);glDisable(GL_SCISSOR_TEST);if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("WebGL frame error");
}
}
