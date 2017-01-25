#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__)

#include <Cocoa/Cocoa.h>
#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "Input/InputSystem.h"
#include "Platform/TemplateMacOS/MouseDeviceMacOS.h"
#include "UI/UIEvent.h"

#if !defined(__DAVAENGINE_COREV2__)


#if defined(__DAVAENGINE_STEAM__)
#include "Platform/Steam.h"
#endif

namespace DAVA
{
MouseDeviceMacOS::MouseDeviceMacOS()
{
#if defined(__DAVAENGINE_STEAM__)
    steamOverlayActivationConnId = Steam::GameOverlayActivated.Connect(this, &MouseDeviceMacOS::OnSteamActivation);
#endif
}

MouseDeviceMacOS::~MouseDeviceMacOS()
{
#if defined(__DAVAENGINE_STEAM__)
    Steam::GameOverlayActivated.Disconnect(steamOverlayActivationConnId);
#endif

    if (blankCursor != nullptr)
    {
        [static_cast<NSCursor*>(blankCursor) release];
    }
}
    
#if defined(__DAVAENGINE_STEAM__)
void MouseDeviceMacOS::OnSteamActivation(bool active)
{
    [NSCursor setHiddenUntilMouseMoves:NO];
}
#endif

void MouseDeviceMacOS::SetMode(eCaptureMode newMode)
{
    switch (newMode)
    {
    case eCaptureMode::OFF:
        OSXShowCursor();
        CGAssociateMouseAndMouseCursorPosition(true);
        break;
    case eCaptureMode::PINING:
        OSXHideCursor();
        MovePointerToWindowCenter();
        CGAssociateMouseAndMouseCursorPosition(false);
        skipMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
        break;
    case eCaptureMode::FRAME:
    // Unsupported yet
    default:
        DVASSERT(false, "Unsupported cursor capture mode");
    }
}

void MouseDeviceMacOS::SetCursorInCenter()
{
    if (blankCursor != nullptr)
    {
        [static_cast<NSCursor*>(blankCursor) set];
    }
}

bool MouseDeviceMacOS::SkipEvents(const UIEvent* event)
{
    bool isMouse = event->device == eInputDevices::MOUSE;
    bool isMovePhase = event->phase == UIEvent::Phase::DRAG || event->phase == UIEvent::Phase::MOVE;

    if (isMouse && isMovePhase && skipMouseMoveEvents != 0)
    {
        skipMouseMoveEvents--;
        return true;
    }

    return false;
}

void MouseDeviceMacOS::MovePointerToWindowCenter()
{
#if defined(__DAVAENGINE_COREV2__)
    NSRect windowRect;
#else
    NSRect windowRect = [[static_cast<NSView*>(Core::Instance()->GetNativeView()) window] frame];
#endif
    NSRect screenRect = [[NSScreen mainScreen] frame];

    // Window origin is at bottom-left edge, but CGWarpMouseCursorPosition requires point in screen coordinates
    windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
    float x = windowRect.origin.x + windowRect.size.width / 2.0f;
    float y = windowRect.origin.y + windowRect.size.height / 2.0f;
    CGWarpMouseCursorPosition(CGPointMake(x, y));
}

void MouseDeviceMacOS::OSXShowCursor()
{
    if (!cursorVisible)
    {
        [[NSCursor arrowCursor] set];
        cursorVisible = true;
    }
}

void MouseDeviceMacOS::OSXHideCursor()
{
    if (cursorVisible)
    {
        [static_cast<NSCursor*>(GetOrCreateBlankCursor()) set];
        cursorVisible = false;
    }
}

void* MouseDeviceMacOS::GetOrCreateBlankCursor()
{
    if (blankCursor != nullptr)
    {
        return blankCursor;
    }

    // Image data -> CGDataProviderRef
    const size_t width = 1;
    const size_t height = 1;
    uint32 pixel = 0;
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(nullptr, &pixel, sizeof(pixel), nullptr);

    // CGDataProviderRef -> CGImageRef
    const size_t bitsPerComponent = 8;
    const size_t bitsPerPixel = 32;
    const size_t bytesPerRow = width * 4;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaLast | kCGBitmapByteOrder32Host;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    CGImageRef imageRef = CGImageCreate(width,
                                        height,
                                        bitsPerComponent,
                                        bitsPerPixel,
                                        bytesPerRow,
                                        colorSpace,
                                        bitmapInfo,
                                        dataProvider,
                                        nullptr,
                                        false,
                                        renderingIntent);

    // CGImageRef -> NSImage
    NSImage* img = [[NSImage alloc] initWithCGImage:imageRef size:NSMakeSize(width, height)];

    // NSImage -> NSCursor
    NSCursor* cursor = [[NSCursor alloc] initWithImage:img hotSpot:NSMakePoint(0, 0)];

    CFRelease(dataProvider);
    CFRelease(colorSpace);
    CGImageRelease(imageRef);
    [img release];

    blankCursor = cursor;
    return blankCursor;
}

} //  namespace DAVA

#endif // !defined(__DAVAENGINE_COREV2__)

#endif // __DAVAENGINE_MACOS__
