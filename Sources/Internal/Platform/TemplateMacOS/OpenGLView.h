#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include "DAVAEngine.h"

@interface OpenGLView : NSOpenGLView
{
    DAVA::Vector<DAVA::UIEvent> allTouches;
}

#ifdef __DAVAENGINE_MACOS_VERSION_10_6__
- (size_t)displayBitsPerPixel:(CGDirectDisplayID)displayId;
#endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__

@end
