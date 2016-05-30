#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__)

#include <Cocoa/Cocoa.h>
#include "Core/Core.h"
#include "FileSystem/FileSystem.h"
#include "Input/InputSystem.h"
#include "Platform/TemplateMacOS/MouseDeviceMacOS.h"

namespace DAVA
{
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
        DVASSERT_MSG(false, "Unsupported cursor capture mode");
    }
}

void MouseDeviceMacOS::SetCursorInCenter()
{
}

bool MouseDeviceMacOS::SkipEvents(const UIEvent* event)
{
    if (event->device == UIEvent::Device::MOUSE)
    {
        if (skipMouseMoveEvents)
        {
            skipMouseMoveEvents--;
            return true;
        }
    }
    return false;
}

void MouseDeviceMacOS::MovePointerToWindowCenter()
{
    NSRect windowRect = [[static_cast<NSView*>(Core::Instance()->GetNativeView()) window] frame];
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
        [NSCursor unhide];
        cursorVisible = true;
    }
}

void MouseDeviceMacOS::OSXHideCursor()
{
    if (cursorVisible)
    {
        [NSCursor hide];
        cursorVisible = false;
    }
}

} //  namespace DAVA

#endif // __DAVAENGINE_MACOS__
