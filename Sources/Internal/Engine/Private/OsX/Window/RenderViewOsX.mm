#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/RenderViewOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSScreen.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSTrackingArea.h>
#import <OpenGL/OpenGL.h>

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#include "Logger/Logger.h"

@implementation RenderView

- (id)initWithFrame:(NSRect)frameRect andBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge;
{
    bridge = nativeBridge;

    uint32_t bitsPerPixel = [self displayBitsPerPixel:kCGDirectMainDisplay];
    // Pixel Format Attributes for the view-based (non-fullscreen) NSOpenGLContext
    NSOpenGLPixelFormatAttribute attrs[] =
    {
      // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.
      // This makes the view-based context a compatible with the fullscreen context, enabling us
      // to use the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
      NSOpenGLPFANoRecovery,
      NSOpenGLPFAColorSize, bitsPerPixel,
      NSOpenGLPFADepthSize, 16,
      NSOpenGLPFAStencilSize, 8,
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFAAccelerated,
      0
    };

    // Create non-fullscreen pixel format.
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    self = [super initWithFrame:frameRect pixelFormat:pixelFormat];

    [self setBackbufferScale:1.0f];

    // Prepare tracking area to receive messages:
    //  - mouseEntered and mouseExited, used with mouse capture handling
    //  - mouseMoved which is delivered only when cursor inside active window
    // clang-format off
    NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                    NSTrackingActiveInKeyWindow |
                                    NSTrackingInVisibleRect |
                                    NSTrackingMouseMoved;
    // clang-format on
    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];
    [self addTrackingArea:trackingArea];

    return self;
}

- (void)reshape
{
    const NSSize frameSize = [self frame].size;
    const DAVA::float32 resultScale = _backbufferScale * [[NSScreen mainScreen] backingScaleFactor];

    const GLint backingSize[2] = { GLint(frameSize.width * resultScale), GLint(frameSize.height * resultScale) };
    CGLSetParameter([[self openGLContext] CGLContextObj], kCGLCPSurfaceBackingSize, backingSize);
    CGLEnable([[self openGLContext] CGLContextObj], kCGLCESurfaceBackingSize);
    CGLUpdateContext([[self openGLContext] CGLContextObj]);
}

- (NSSize)surfaceSize
{
    GLint customScaleEnabled = 0;
    CGLIsEnabled([[self openGLContext] CGLContextObj], kCGLCESurfaceBackingSize, &customScaleEnabled);

    // If SurfaceBackingSize is enabled - calculate custom size
    // Otherwise it's the same as frontbuffer
    if (customScaleEnabled)
    {
        GLint backingSize[2];
        CGLGetParameter([[self openGLContext] CGLContextObj], kCGLCPSurfaceBackingSize, backingSize);

        NSSize result;
        result.width = backingSize[0];
        result.height = backingSize[1];

        return result;
    }
    else
    {
        return [self frame].size;
    }
}

- (uint32_t)displayBitsPerPixel:(CGDirectDisplayID)displayId
{
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
    uint32_t bitsPerPixel = 0;

    CFStringRef pixelEncoding = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(pixelEncoding, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        bitsPerPixel = 32;
    }
    else if (CFStringCompare(pixelEncoding, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        bitsPerPixel = 16;
    }
    else if (CFStringCompare(pixelEncoding, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        bitsPerPixel = 8;
    }

    CGDisplayModeRelease(mode);
    CFRelease(pixelEncoding);
    return bitsPerPixel;
}

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseMoved:(NSEvent*)theEvent
{
    bridge->MouseMove(theEvent);
}

- (void)mouseEntered:(NSEvent*)theEvent
{
    bridge->MouseEntered(theEvent);
}

- (void)mouseExited:(NSEvent*)theEvent
{
    bridge->MouseExited(theEvent);
}

- (void)scrollWheel:(NSEvent*)theEvent
{
    bridge->MouseWheel(theEvent);
}

- (void)mouseDown:(NSEvent*)theEvent
{
    bridge->MouseClick(theEvent);
}

- (void)mouseUp:(NSEvent*)theEvent
{
    bridge->MouseClick(theEvent);
}

- (void)mouseDragged:(NSEvent*)theEvent
{
    bridge->MouseMove(theEvent);
}

- (void)rightMouseDown:(NSEvent*)theEvent
{
    bridge->MouseClick(theEvent);
}

- (void)rightMouseUp:(NSEvent*)theEvent
{
    bridge->MouseClick(theEvent);
}

- (void)rightMouseDragged:(NSEvent*)theEvent
{
    bridge->MouseMove(theEvent);
}

- (void)otherMouseDown:(NSEvent*)theEvent
{
    bridge->MouseClick(theEvent);
}

- (void)otherMouseUp:(NSEvent*)theEvent
{
    bridge->MouseClick(theEvent);
}

- (void)otherMouseDragged:(NSEvent*)theEvent
{
    bridge->MouseMove(theEvent);
}

- (void)keyDown:(NSEvent*)theEvent
{
    bridge->KeyEvent(theEvent);
}

- (void)keyUp:(NSEvent*)theEvent
{
    bridge->KeyEvent(theEvent);
}

- (void)flagsChanged:(NSEvent*)theEvent
{
    bridge->FlagsChanged(theEvent);
}

- (void)magnifyWithEvent:(NSEvent*)theEvent
{
    bridge->MagnifyWithEvent(theEvent);
}

- (void)rotateWithEvent:(NSEvent*)theEvent
{
    bridge->RotateWithEvent(theEvent);
}

- (void)swipeWithEvent:(NSEvent*)theEvent
{
    bridge->SwipeWithEvent(theEvent);
}

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
