#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include "DAVAEngine.h"
//#import "TScene.h"

using namespace DAVA;

@interface OpenGLView : NSOpenGLView
{
    Vector<DAVA::UIEvent> allTouches;

    NSTrackingArea* trackingArea;
}

#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
- (size_t)displayBitsPerPixel:(CGDirectDisplayID)displayId;
#endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__

- (void)enableTrackingArea;
- (void)disableTrackingArea;

@end
