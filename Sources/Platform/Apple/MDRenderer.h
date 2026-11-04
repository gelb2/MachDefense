#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#include "../../Core/RenderTypes.hpp"

struct MDRenderMetrics {
    NSUInteger sprites=0,drawCalls=0,textureBinds=0,filterChanges=0,blendChanges=0,clipChanges=0;
    double submitMilliseconds=0,presentMilliseconds=0,totalMilliseconds=0;
};
@interface MDRenderer : NSObject
- (instancetype)initWithResourceRoot:(NSURL*)root;
- (BOOL)drawFrame:(const md::Frame&)frame toLayer:(CAEAGLLayer*)layer;
- (md::Vec2)imageSize:(const std::string&)name;
- (BOOL)logicalPoint:(CGPoint)point result:(md::Vec2*)result;
- (void)discardDrawable;
- (void)finish;
// Diagnostic GPU readback only. Production presentation never invokes it.
- (NSData*)readFramebufferRGBA;
- (CGSize)drawablePixelSize;
- (MDRenderMetrics)lastFrameMetrics;
@property(nonatomic,readonly) NSUInteger presentedFrames;
@property(nonatomic,readonly) NSUInteger textureUploads;
@property(nonatomic,readonly) NSString* lastError;
@end
