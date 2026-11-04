#import "MDRenderer.h"
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "../../../Modules/ThirdParty/stb/stb_image.h"

namespace {
struct Texture { GLuint id=0; int width=0,height=0,storageWidth=0,storageHeight=0; };
bool validRect(md::Rect r){return std::isfinite(r.x)&&std::isfinite(r.y)&&std::isfinite(r.width)&&std::isfinite(r.height);}
}
@implementation MDRenderer {
    EAGLContext* _context;
    NSURL* _root;
    GLuint _framebuffer,_renderbuffer;
    GLint _width,_height;
    CGSize _pointSize;
    CGFloat _scale;
    CGRect _viewport;
    float _logicalWidth,_logicalHeight;
    std::map<std::string,Texture> _textures;
    std::map<std::string,md::Vec2> _imageSizes;
    std::set<std::string> _failedTextures;
    MDRenderMetrics _metrics;
}
- (instancetype)initWithResourceRoot:(NSURL*)root {
    if(!(self=[super init]))return nil;
    _root=root;_context=[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    if(!_context)return nil;
    return self;
}
- (void)fail:(NSString*)message {_lastError=message;NSLog(@"MachDefense GPU: %@",message);}
- (void)dealloc {
    EAGLContext* previous=EAGLContext.currentContext;
    if([EAGLContext setCurrentContext:_context]){
        for(auto& [name,t]:_textures)glDeleteTextures(1,&t.id);
        glDeleteFramebuffersOES(1,&_framebuffer);glDeleteRenderbuffersOES(1,&_renderbuffer);
        [EAGLContext setCurrentContext:previous==_context?nil:previous];
    }
}
- (Texture*)texture:(const std::string&)name premultiplied:(BOOL)premultiplied {
    const std::string key=premultiplied?name+"#premultiplied":name;
    auto found=_textures.find(key);if(found!=_textures.end())return &found->second;
    if(_failedTextures.contains(name))return nullptr;
    if(name.empty()||name.front()=='/'||name.find("..")!=std::string::npos)return nullptr;
    NSString* relative=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
    NSURL* url=name.starts_with("Generated/")?[_root URLByAppendingPathComponent:relative]:[[_root URLByAppendingPathComponent:@"Images"] URLByAppendingPathComponent:relative];
    if(!url)return nullptr;
    Texture t;int channels=0;
    stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
    unsigned char* pixels=stbi_load(url.fileSystemRepresentation,&t.width,&t.height,&channels,4);
    if(!pixels){_failedTextures.insert(name);[self fail:[NSString stringWithFormat:@"Cannot decode %@",relative]];return nullptr;}
    GLint maximum=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum);
    if(t.width<=0||t.height<=0||t.width>maximum||t.height>maximum){stbi_image_free(pixels);[self fail:@"Texture exceeds GPU limit"];return nullptr;}
    _imageSizes[name]={float(t.width),float(t.height)};
    if(premultiplied)for(std::size_t i=0;i<std::size_t(t.width)*t.height;++i){auto* p=pixels+i*4;for(int channel=0;channel<3;++channel)p[channel]=static_cast<unsigned char>(float(p[channel])*(float(p[3])/255.f));}
    // Texture2D(Image)::initWithImage 0x4b590 pads to powers of two and
    // uploads cleared pixels outside the original image. Preserve its edge sampling.
    t.storageWidth=1;t.storageHeight=1;
    while(t.storageWidth<t.width)t.storageWidth*=2;
    while(t.storageHeight<t.height)t.storageHeight*=2;
    if(t.storageWidth>maximum||t.storageHeight>maximum){stbi_image_free(pixels);[self fail:@"Padded texture exceeds GPU limit"];return nullptr;}
    std::vector<unsigned char> padded(std::size_t(t.storageWidth)*t.storageHeight*4,0);
    for(int y=0;y<t.height;++y)std::copy_n(pixels+std::size_t(y)*t.width*4,std::size_t(t.width)*4,padded.data()+std::size_t(y)*t.storageWidth*4);
    glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t.storageWidth,t.storageHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,padded.data());
    stbi_image_free(pixels);
    if(glGetError()!=GL_NO_ERROR){glDeleteTextures(1,&t.id);[self fail:@"Texture upload failed"];return nullptr;}
    ++_textureUploads;return &_textures.emplace(key,t).first->second;
}
- (md::Vec2)imageSize:(const std::string&)name {
    auto found=_textures.find(name);if(found!=_textures.end())return {float(found->second.width),float(found->second.height)};
    auto cached=_imageSizes.find(name);if(cached!=_imageSizes.end())return cached->second;
    if(name.empty()||name.front()=='/'||name.find("..")!=std::string::npos)return {};
    NSString* relative=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
    NSURL* url=name.starts_with("Generated/")?[_root URLByAppendingPathComponent:relative]:[[_root URLByAppendingPathComponent:@"Images"] URLByAppendingPathComponent:relative];
    int w=0,h=0,c=0;
    md::Vec2 size;if(stbi_info(url.fileSystemRepresentation,&w,&h,&c))size={float(w),float(h)};
    _imageSizes.emplace(name,size);return size;
}
- (BOOL)allocate:(CAEAGLLayer*)layer {
    if(_renderbuffer&&CGSizeEqualToSize(_pointSize,layer.bounds.size)&&_scale==layer.contentsScale)return YES;
    GLuint buffer=0,fbo=0;glGenRenderbuffersOES(1,&buffer);glBindRenderbufferOES(GL_RENDERBUFFER_OES,buffer);
    if(![_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:layer]){glDeleteRenderbuffersOES(1,&buffer);[self fail:@"Drawable allocation failed"];return NO;}
    GLint w=0,h=0;glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES,GL_RENDERBUFFER_WIDTH_OES,&w);glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES,GL_RENDERBUFFER_HEIGHT_OES,&h);
    glGenFramebuffersOES(1,&fbo);glBindFramebufferOES(GL_FRAMEBUFFER_OES,fbo);glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES,GL_COLOR_ATTACHMENT0_OES,GL_RENDERBUFFER_OES,buffer);
    if(w<=0||h<=0||glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES)!=GL_FRAMEBUFFER_COMPLETE_OES){glDeleteRenderbuffersOES(1,&buffer);glDeleteFramebuffersOES(1,&fbo);[self fail:@"Incomplete drawable"];return NO;}
    glDeleteFramebuffersOES(1,&_framebuffer);glDeleteRenderbuffersOES(1,&_renderbuffer);
    _framebuffer=fbo;_renderbuffer=buffer;_width=w;_height=h;_pointSize=layer.bounds.size;_scale=layer.contentsScale;return YES;
}
- (BOOL)drawFrame:(const md::Frame&)frame toLayer:(CAEAGLLayer*)layer {
    if(!(std::isfinite(frame.logicalWidth)&&std::isfinite(frame.logicalHeight)&&frame.logicalWidth>0&&frame.logicalHeight>0))return NO;
    const double started=CACurrentMediaTime();_metrics={};_metrics.sprites=frame.sprites.size();
    EAGLContext* previous=EAGLContext.currentContext;
    if(![EAGLContext setCurrentContext:_context])return NO;
    if(![self allocate:layer]){[EAGLContext setCurrentContext:previous];return NO;}
    _logicalWidth=frame.logicalWidth;_logicalHeight=frame.logicalHeight;
    const float factor=std::min(_width/frame.logicalWidth,_height/frame.logicalHeight);
    const GLint vw=GLint(std::round(frame.logicalWidth*factor)),vh=GLint(std::round(frame.logicalHeight*factor));
    const GLint vx=(_width-vw)/2,vy=(_height-vh)/2;
    const float sx=vw/frame.logicalWidth,sy=vh/frame.logicalHeight;
    _viewport=CGRectMake(vx/_scale,(_height-vy-vh)/_scale,vw/_scale,vh/_scale);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES,_framebuffer);glViewport(0,0,_width,_height);glDisable(GL_SCISSOR_TEST);
    glClearColor(frame.background.r,frame.background.g,frame.background.b,frame.background.a);glClear(GL_COLOR_BUFFER_BIT);
    glViewport(vx,vy,vw,vh);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrthof(0,frame.logicalWidth,frame.logicalHeight,0,-1,1);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();glMatrixMode(GL_TEXTURE);glLoadIdentity();glMatrixMode(GL_MODELVIEW);
    glActiveTexture(GL_TEXTURE0);glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_VERTEX_ARRAY);glEnableClientState(GL_TEXTURE_COORD_ARRAY);glDisableClientState(GL_COLOR_ARRAY);
    BOOL valid=YES;
    std::vector<const md::TextureMesh*> meshes;meshes.reserve(frame.meshes.size());
    for(const auto& mesh:frame.meshes)meshes.push_back(&mesh);
    std::stable_sort(meshes.begin(),meshes.end(),[](const auto* a,const auto* b){return a->beforeSpriteIndex<b->beforeSpriteIndex;});
    auto drawMesh=[&](const md::TextureMesh& mesh){
        if(mesh.vertices.size()<3)return;
        for(const auto& v:mesh.vertices)if(!std::isfinite(v.position.x)||!std::isfinite(v.position.y)||!std::isfinite(v.uv.x)||!std::isfinite(v.uv.y))return;
        const bool premultiplied=mesh.blend==md::Blend::premultipliedAlpha;
        Texture* texture=[self texture:mesh.image premultiplied:premultiplied];if(!texture){valid=NO;return;}
        glDisable(GL_SCISSOR_TEST);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture->id);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
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
            Texture* t=[self texture:s.image premultiplied:premultiplied];if(!t){valid=NO;continue;}
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
    glDisable(GL_SCISSOR_TEST);glBindRenderbufferOES(GL_RENDERBUFFER_OES,_renderbuffer);
    const double submitted=CACurrentMediaTime();
    BOOL ok=[_context presentRenderbuffer:GL_RENDERBUFFER_OES];
    const double presented=CACurrentMediaTime();
    _metrics.submitMilliseconds=(submitted-started)*1000;_metrics.presentMilliseconds=(presented-submitted)*1000;_metrics.totalMilliseconds=(presented-started)*1000;
    const GLenum error=glGetError();if(error!=GL_NO_ERROR){[self fail:[NSString stringWithFormat:@"OpenGL error 0x%x",error]];ok=NO;}
    if(ok)++_presentedFrames;[EAGLContext setCurrentContext:previous];return ok&&valid;
}
- (BOOL)logicalPoint:(CGPoint)p result:(md::Vec2*)out {
    if(!out||_viewport.size.width<=0||_viewport.size.height<=0||!CGRectContainsPoint(_viewport,p))return NO;
    *out={float((p.x-_viewport.origin.x)*_logicalWidth/_viewport.size.width),float((p.y-_viewport.origin.y)*_logicalHeight/_viewport.size.height)};return YES;
}
- (void)discardDrawable {
    EAGLContext* previous=EAGLContext.currentContext;if([EAGLContext setCurrentContext:_context]){
        glDeleteFramebuffersOES(1,&_framebuffer);glDeleteRenderbuffersOES(1,&_renderbuffer);_framebuffer=0;_renderbuffer=0;[EAGLContext setCurrentContext:previous];
    }
}
- (void)finish {EAGLContext* p=EAGLContext.currentContext;if([EAGLContext setCurrentContext:_context]){glFinish();[EAGLContext setCurrentContext:p];}}
- (NSData*)readFramebufferRGBA {
    if(!_framebuffer||_width<=0||_height<=0)return nil;
    EAGLContext* previous=EAGLContext.currentContext;if(![EAGLContext setCurrentContext:_context])return nil;
    glBindFramebufferOES(GL_FRAMEBUFFER_OES,_framebuffer);glPixelStorei(GL_PACK_ALIGNMENT,1);
    NSMutableData* data=[NSMutableData dataWithLength:std::size_t(_width)*_height*4];
    glReadPixels(0,0,_width,_height,GL_RGBA,GL_UNSIGNED_BYTE,data.mutableBytes);
    BOOL ok=glGetError()==GL_NO_ERROR;[EAGLContext setCurrentContext:previous];return ok?data:nil;
}
- (MDRenderMetrics)lastFrameMetrics{return _metrics;}
- (CGSize)drawablePixelSize{return CGSizeMake(_width,_height);}
@end
