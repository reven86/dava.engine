#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
MainDispatcherEvent MainDispatcherEvent::CreateAppTerminateEvent(bool triggeredBySystem)
{
    MainDispatcherEvent e(APP_TERMINATE);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.terminateEvent.triggeredBySystem = triggeredBySystem;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateUserCloseRequestEvent(Window* window)
{
    MainDispatcherEvent e(USER_CLOSE_REQUEST, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowCreatedEvent(Window* window, float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    MainDispatcherEvent e(WINDOW_CREATED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.sizeEvent.width = width;
    e.sizeEvent.height = height;
    e.sizeEvent.scaleX = scaleX;
    e.sizeEvent.scaleY = scaleY;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowDestroyedEvent(Window* window)
{
    MainDispatcherEvent e(WINDOW_DESTROYED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowSizeChangedEvent(Window* window, float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    MainDispatcherEvent e(WINDOW_SIZE_SCALE_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.sizeEvent.width = width;
    e.sizeEvent.height = height;
    e.sizeEvent.scaleX = scaleX;
    e.sizeEvent.scaleY = scaleY;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowFocusChangedEvent(Window* window, bool focusState)
{
    MainDispatcherEvent e(WINDOW_FOCUS_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.stateEvent.state = focusState;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowVisibilityChangedEvent(Window* window, bool visibilityState)
{
    MainDispatcherEvent e(WINDOW_VISIBILITY_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.stateEvent.state = visibilityState;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowKeyPressEvent(Window* window, eType keyEventType, uint32 key, eModifierKeys modifierKeys, bool isRepeated)
{
    DVASSERT(keyEventType == KEY_DOWN || keyEventType == KEY_UP || keyEventType == KEY_CHAR);

    MainDispatcherEvent e(keyEventType, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.modifierKeys = modifierKeys;
    e.keyEvent.isRepeated = isRepeated;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowMouseClickEvent(Window* window, eType mouseClickEventType, eMouseButtons button, float32 x, float32 y, uint32 clicks, eModifierKeys modifierKeys, bool isRelative)
{
    DVASSERT(mouseClickEventType == MOUSE_BUTTON_DOWN || mouseClickEventType == MOUSE_BUTTON_UP);
    DVASSERT(button != eMouseButtons::NONE);

    MainDispatcherEvent e(mouseClickEventType, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.mouseEvent.button = button;
    e.mouseEvent.clicks = clicks;
    e.mouseEvent.modifierKeys = modifierKeys;
    e.mouseEvent.x = x;
    e.mouseEvent.y = y;
    e.mouseEvent.scrollDeltaX = 0.f;
    e.mouseEvent.scrollDeltaY = 0.f;
    e.mouseEvent.isRelative = isRelative;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowMouseMoveEvent(Window* window, float32 x, float32 y, eModifierKeys modifierKeys, bool isRelative)
{
    MainDispatcherEvent e(MOUSE_MOVE, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.mouseEvent.button = eMouseButtons::NONE;
    e.mouseEvent.clicks = 0;
    e.mouseEvent.modifierKeys = modifierKeys;
    e.mouseEvent.x = x;
    e.mouseEvent.y = y;
    e.mouseEvent.scrollDeltaX = 0.f;
    e.mouseEvent.scrollDeltaY = 0.f;
    e.mouseEvent.isRelative = isRelative;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowMouseWheelEvent(Window* window, float32 x, float32 y, float32 deltaX, float32 deltaY, eModifierKeys modifierKeys, bool isRelative)
{
    MainDispatcherEvent e(MOUSE_WHEEL, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.mouseEvent.button = eMouseButtons::NONE;
    e.mouseEvent.clicks = 0;
    e.mouseEvent.modifierKeys = modifierKeys;
    e.mouseEvent.x = x;
    e.mouseEvent.y = y;
    e.mouseEvent.scrollDeltaX = deltaX;
    e.mouseEvent.scrollDeltaY = deltaY;
    e.mouseEvent.isRelative = isRelative;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowTouchEvent(Window* window, eType touchEventType, uint32 touchId, float32 x, float32 y, eModifierKeys modifierKeys)
{
    DVASSERT(touchEventType == TOUCH_DOWN || touchEventType == TOUCH_UP || touchEventType == TOUCH_MOVE);

    MainDispatcherEvent e(touchEventType, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.touchEvent.touchId = touchId;
    e.touchEvent.modifierKeys = modifierKeys;
    e.touchEvent.x = x;
    e.touchEvent.y = y;
    return e;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
