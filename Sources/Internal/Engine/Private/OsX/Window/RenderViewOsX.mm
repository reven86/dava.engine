#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/Window/RenderViewOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSOpenGL.h>
#import <OpenGL/OpenGL.h>

#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#include "Logger/Logger.h"

@implementation OpenGLViewOsX

- (id)initWithFrame:(NSRect)frameRect bridge:(DAVA::Private::WindowNativeBridgeOsX*)nativeBridge;
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
    // Enable retina resolution
    [self setWantsBestResolutionOpenGLSurface:YES];
    return self;
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
}

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
