#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
bool MainDispatcherEvent::IsInputEvent(eType type)
{
    return (FIRST_INPUT_EVENT <= type && type <= LAST_INPUT_EVENT);
}

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

MainDispatcherEvent MainDispatcherEvent::CreateGamepadAddedEvent(uint32 deviceId)
{
    MainDispatcherEvent e(GAMEPAD_ADDED);
    e.gamepadEvent.deviceId = deviceId;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateGamepadRemovedEvent(uint32 deviceId)
{
    MainDispatcherEvent e(GAMEPAD_REMOVED);
    e.gamepadEvent.deviceId = deviceId;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateGamepadMotionEvent(uint32 deviceId, uint32 axis, float32 value)
{
    MainDispatcherEvent e(GAMEPAD_MOTION);
    e.gamepadEvent.deviceId = deviceId;
    e.gamepadEvent.axis = axis;
    e.gamepadEvent.value = value;
    e.gamepadEvent.button = 0;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateDisplayConfigChangedEvent(DisplayInfo* displayInfo, size_t count)
{
    DVASSERT(displayInfo != nullptr && count > 0);

    MainDispatcherEvent e(DISPLAY_CONFIG_CHANGED);
    e.displayConfigEvent.displayInfo = displayInfo;
    e.displayConfigEvent.count = count;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateGamepadButtonEvent(uint32 deviceId, eType gamepadButtonEventType, uint32 button)
{
    DVASSERT(gamepadButtonEventType == GAMEPAD_BUTTON_DOWN || gamepadButtonEventType == GAMEPAD_BUTTON_UP);

    MainDispatcherEvent e(gamepadButtonEventType);
    e.gamepadEvent.deviceId = deviceId;
    e.gamepadEvent.button = button;
    e.gamepadEvent.axis = 0;
    e.gamepadEvent.value = 0;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowCreatedEvent(Window* window, float32 w, float32 h, float32 surfaceW, float32 surfaceH, float32 dpi, eFullscreen fullscreen)
{
    MainDispatcherEvent e(WINDOW_CREATED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.sizeEvent.width = w;
    e.sizeEvent.height = h;
    e.sizeEvent.surfaceWidth = surfaceW;
    e.sizeEvent.surfaceHeight = surfaceH;
    e.sizeEvent.surfaceScale = 1.0f;
    e.sizeEvent.dpi = dpi;
    e.sizeEvent.fullscreen = fullscreen;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowDestroyedEvent(Window* window)
{
    MainDispatcherEvent e(WINDOW_DESTROYED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowSizeChangedEvent(Window* window, float32 w, float32 h, float32 surfaceW, float32 surfaceH, float32 surfaceScale, float32 dpi, eFullscreen fullscreen)
{
    MainDispatcherEvent e(WINDOW_SIZE_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.sizeEvent.width = w;
    e.sizeEvent.height = h;
    e.sizeEvent.surfaceWidth = surfaceW;
    e.sizeEvent.surfaceHeight = surfaceH;
    e.sizeEvent.surfaceScale = surfaceScale;
    e.sizeEvent.dpi = dpi;
    e.sizeEvent.fullscreen = fullscreen;
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

MainDispatcherEvent MainDispatcherEvent::CreateWindowDpiChangedEvent(Window* window, float32 dpi)
{
    MainDispatcherEvent e(WINDOW_DPI_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.dpiEvent.dpi = dpi;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowCancelInputEvent(Window* window)
{
    MainDispatcherEvent e(WINDOW_CANCEL_INPUT, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
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

MainDispatcherEvent MainDispatcherEvent::CreateWindowMagnificationGestureEvent(Window* window, float32 magnification, eModifierKeys modifierKeys)
{
    MainDispatcherEvent e(TRACKPAD_GESTURE, window);
    e.trackpadGestureEvent.magnification = magnification;
    e.trackpadGestureEvent.rotation = 0;
    e.trackpadGestureEvent.deltaX = 0;
    e.trackpadGestureEvent.deltaY = 0;
    e.trackpadGestureEvent.modifierKeys = modifierKeys;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowRotationGestureEvent(Window* window, float32 rotation, eModifierKeys modifierKeys)
{
    MainDispatcherEvent e(TRACKPAD_GESTURE, window);
    e.trackpadGestureEvent.magnification = 0;
    e.trackpadGestureEvent.rotation = rotation;
    e.trackpadGestureEvent.deltaX = 0;
    e.trackpadGestureEvent.deltaY = 0;
    e.trackpadGestureEvent.modifierKeys = modifierKeys;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowSwipeGestureEvent(Window* window, float32 deltaX, float32 deltaY, eModifierKeys modifierKeys)
{
    MainDispatcherEvent e(TRACKPAD_GESTURE, window);
    e.trackpadGestureEvent.magnification = 0;
    e.trackpadGestureEvent.rotation = 0;
    e.trackpadGestureEvent.deltaX = deltaX;
    e.trackpadGestureEvent.deltaY = deltaY;
    e.trackpadGestureEvent.modifierKeys = modifierKeys;
    return e;
}

MainDispatcherEvent MainDispatcherEvent::CreateWindowCaptureLostEvent(Window* window)
{
    MainDispatcherEvent e(WINDOW_CAPTURE_LOST, window);
    return e;
}
} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
