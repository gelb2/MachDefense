#include "Renderer.hpp"
#include "stb_image.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace md::android {
namespace {bool validRect(Rect r){return std::isfinite(r.x)&&std::isfinite(r.y)&&std::isfinite(r.width)&&std::isfinite(r.height);}}
Texture* Renderer::texture(const std::string& name,bool premultiplied){
    const auto key=premultiplied?name+"#premultiplied":name;
    if(auto it=_textures.find(key);it!=_textures.end())return &it->second;
    auto bytes=assets_.read(Assets::imagePath(name));Texture t;int channels=0;
    stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
    auto* pixels=stbi_load_from_memory(bytes.data(),int(bytes.size()),&t.width,&t.height,&channels,4);
    if(!pixels)throw std::runtime_error("Cannot decode image: "+name);
    struct Free{unsigned char* p;~Free(){stbi_image_free(p);}} free{pixels};
    GLint maximum=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum);
    if(t.width<=0||t.height<=0||t.width>maximum||t.height>maximum)throw std::runtime_error("Texture exceeds GPU limit");
    if(premultiplied)for(std::size_t i=0;i<std::size_t(t.width)*t.height;++i){auto* p=pixels+i*4;for(int c=0;c<3;++c)p[c]=static_cast<unsigned char>(float(p[c])*(float(p[3])/255.f));}
    t.storageWidth=1;t.storageHeight=1;while(t.storageWidth<t.width)t.storageWidth*=2;while(t.storageHeight<t.height)t.storageHeight*=2;
    if(t.storageWidth>maximum||t.storageHeight>maximum)throw std::runtime_error("Padded texture exceeds GPU limit");
    std::vector<unsigned char> padded(std::size_t(t.storageWidth)*t.storageHeight*4,0);
    for(int y=0;y<t.height;++y)std::copy_n(pixels+std::size_t(y)*t.width*4,std::size_t(t.width)*4,padded.data()+std::size_t(y)*t.storageWidth*4);
    glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t.storageWidth,t.storageHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,padded.data());
    if(glGetError()!=GL_NO_ERROR){glDeleteTextures(1,&t.id);throw std::runtime_error("Texture upload failed: "+name);}
    ++textureUploads;return &_textures.emplace(key,t).first->second;
}
void Renderer::release(){for(auto& [n,t]:_textures)glDeleteTextures(1,&t.id);_textures.clear();}
bool Renderer::draw(const Frame& frame,int width,int height){
    if(!(std::isfinite(frame.logicalWidth)&&std::isfinite(frame.logicalHeight)&&frame.logicalWidth>0&&frame.logicalHeight>0))return false;
    _metrics={};_metrics.sprites=frame.sprites.size();_width=width;_height=height;_logicalWidth=frame.logicalWidth;_logicalHeight=frame.logicalHeight;
    const float factor=std::min(_width/frame.logicalWidth,_height/frame.logicalHeight);
    const GLint vw=GLint(std::round(frame.logicalWidth*factor)),vh=GLint(std::round(frame.logicalHeight*factor));
    const GLint vx=(_width-vw)/2,vy=(_height-vh)/2;
    const float sx=vw/frame.logicalWidth,sy=vh/frame.logicalHeight;
    viewport_={float(vx),float(_height-vy-vh),float(vw),float(vh)};
    glViewport(0,0,_width,_height);glDisable(GL_SCISSOR_TEST);
    glClearColor(frame.background.r,frame.background.g,frame.background.b,frame.background.a);glClear(GL_COLOR_BUFFER_BIT);
    glViewport(vx,vy,vw,vh);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrthof(0,frame.logicalWidth,frame.logicalHeight,0,-1,1);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();glMatrixMode(GL_TEXTURE);glLoadIdentity();glMatrixMode(GL_MODELVIEW);
    glActiveTexture(GL_TEXTURE0);glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_VERTEX_ARRAY);glEnableClientState(GL_TEXTURE_COORD_ARRAY);glDisableClientState(GL_COLOR_ARRAY);
    bool valid=true;
    std::vector<const md::TextureMesh*> meshes;meshes.reserve(frame.meshes.size());
    for(const auto& mesh:frame.meshes)meshes.push_back(&mesh);
    std::stable_sort(meshes.begin(),meshes.end(),[](const auto* a,const auto* b){return a->beforeSpriteIndex<b->beforeSpriteIndex;});
    auto drawMesh=[&](const md::TextureMesh& mesh){
        if(mesh.vertices.size()<3)return;
        for(const auto& v:mesh.vertices)if(!std::isfinite(v.position.x)||!std::isfinite(v.position.y)||!std::isfinite(v.uv.x)||!std::isfinite(v.uv.y))return;
        const bool premultiplied=mesh.blend==md::Blend::premultipliedAlpha;
        Texture* texture=this->texture(mesh.image,premultiplied);if(!texture){valid=false;return;}
        glDisable(GL_SCISSOR_TEST);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture->id);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,mesh.nearest?GL_NEAREST:GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,mesh.nearest?GL_NEAREST:GL_LINEAR);glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        if(mesh.blend==md::Blend::opaque)glDisable(GL_BLEND);else{glEnable(GL_BLEND);glBlendFunc(premultiplied?GL_ONE:GL_SRC_ALPHA,mesh.blend==md::Blend::additive?GL_ONE:GL_ONE_MINUS_SRC_ALPHA);}
        std::vector<md::MeshVertex> vertices=mesh.vertices;
        for(auto& v:vertices){v.uv.x*=float(texture->width)/texture->storageWidth;v.uv.y*=float(texture->height)/texture->storageHeight;}
        if(premultiplied)for(auto& v:vertices){v.color.r*=v.color.a;v.color.g*=v.color.a;v.color.b*=v.color.a;}
        glLoadIdentity();glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(2,GL_FLOAT,sizeof(md::MeshVertex),&vertices.front().position.x);
        glTexCoordPointer(2,GL_FLOAT,sizeof(md::MeshVertex),&vertices.front().uv.x);
        glColorPointer(4,GL_FLOAT,sizeof(md::MeshVertex),&vertices.front().color.r);
        glDrawArrays(GL_TRIANGLE_STRIP,0,GLsizei(vertices.size()));glDisableClientState(GL_COLOR_ARRAY);
        ++_metrics.drawCalls;++_metrics.textureBinds;++_metrics.filterChanges;++_metrics.blendChanges;++_metrics.clipChanges;
    };
    std::size_t meshIndex=0;
    for(std::size_t spriteIndex=0;spriteIndex<=frame.sprites.size();++spriteIndex){
        while(meshIndex<meshes.size()&&meshes[meshIndex]->beforeSpriteIndex<=spriteIndex)drawMesh(*meshes[meshIndex++]);
        if(spriteIndex==frame.sprites.size())break;
        const auto& s=frame.sprites[spriteIndex];
        if(!validRect(s.destination)||!std::isfinite(s.rotationDegrees)||!std::isfinite(s.pivot.x)||!std::isfinite(s.pivot.y))continue;
        if(s.destination.width==0||s.destination.height==0)continue;
        if(s.clip){
            const auto c=*s.clip;if(!validRect(c))continue;
            const float left=std::clamp(c.x,0.f,frame.logicalWidth),right=std::clamp(c.x+std::max(0.f,c.width),0.f,frame.logicalWidth);
            const float top=std::clamp(c.y,0.f,frame.logicalHeight),bottom=std::clamp(c.y+std::max(0.f,c.height),0.f,frame.logicalHeight);
            glEnable(GL_SCISSOR_TEST);glScissor(vx+GLint(left*sx),vy+GLint((frame.logicalHeight-bottom)*sy),GLsizei(std::max(0.f,(right-left)*sx)),GLsizei(std::max(0.f,(bottom-top)*sy)));
        }else glDisable(GL_SCISSOR_TEST);
        ++_metrics.blendChanges;++_metrics.clipChanges;
        switch(s.blend){case md::Blend::opaque:glDisable(GL_BLEND);break;
            case md::Blend::alpha:glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);break;
            case md::Blend::premultipliedAlpha:glEnable(GL_BLEND);glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);break;
            case md::Blend::additive:glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);break;}
        const bool premultiplied=s.blend==md::Blend::premultipliedAlpha;
        glColor4f(s.color.r*(premultiplied?s.color.a:1),s.color.g*(premultiplied?s.color.a:1),s.color.b*(premultiplied?s.color.a:1),s.color.a);
        const GLfloat w=s.destination.width,h=s.destination.height;
        const GLfloat vertices[]={0,0,w,0,0,h,w,h};
        GLfloat uv[]={0,0,1,0,0,1,1,1};
        if(!s.image.empty()){
            Texture* t=this->texture(s.image,premultiplied);if(!t){valid=false;continue;}
            ++_metrics.textureBinds;++_metrics.filterChanges;
            glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,t->id);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,s.nearest?GL_NEAREST:GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,s.nearest?GL_NEAREST:GL_LINEAR);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
            md::Rect r=s.sourcePx.value_or(md::Rect{0,0,float(t->width),float(t->height)});if(!validRect(r))continue;
            float l=r.x/t->storageWidth,rr=(r.x+r.width)/t->storageWidth,top=r.y/t->storageHeight,b=(r.y+r.height)/t->storageHeight;
            if(s.flipX)std::swap(l,rr);if(s.flipY)std::swap(top,b);
            uv[0]=l;uv[1]=top;uv[2]=rr;uv[3]=top;uv[4]=l;uv[5]=b;uv[6]=rr;uv[7]=b;
        }else glDisable(GL_TEXTURE_2D);
        glLoadIdentity();glTranslatef(s.destination.x+w*s.pivot.x,s.destination.y+h*s.pivot.y,0);glRotatef(s.rotationDegrees,0,0,1);glTranslatef(-w*s.pivot.x,-h*s.pivot.y,0);
        glVertexPointer(2,GL_FLOAT,0,vertices);glTexCoordPointer(2,GL_FLOAT,0,uv);glDrawArrays(GL_TRIANGLE_STRIP,0,4);++_metrics.drawCalls;
    }
    while(meshIndex<meshes.size())drawMesh(*meshes[meshIndex++]);
    glDisable(GL_SCISSOR_TEST);
    if(glGetError()!=GL_NO_ERROR)throw std::runtime_error("OpenGL draw failed");return valid;
}
std::optional<Vec2> Renderer::logicalPoint(float x,float y)const{
    if(viewport_.width<=0||viewport_.height<=0||x<viewport_.x||y<viewport_.y||x>=viewport_.x+viewport_.width||y>=viewport_.y+viewport_.height)return {};
    return Vec2{(x-viewport_.x)*_logicalWidth/viewport_.width,(y-viewport_.y)*_logicalHeight/viewport_.height};
}
}
