#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

@protocol ESRenderer<NSObject>

- (void)startRendering;
- (void)endRendering;
- (BOOL)resizeFromLayer:(CAEAGLLayer*)layer;

- (void)setCurrentContext;
@end
