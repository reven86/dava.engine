#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
struct MainDispatcherEvent final
{
    enum eType : int32
    {
        DUMMY = 0,
        WINDOW_CREATED,
        WINDOW_DESTROYED,
        WINDOW_FOCUS_CHANGED,
        WINDOW_VISIBILITY_CHANGED,
        WINDOW_SIZE_SCALE_CHANGED,

        MOUSE_BUTTON_DOWN,
        MOUSE_BUTTON_UP,
        MOUSE_WHEEL,
        MOUSE_MOVE,

        TOUCH_DOWN,
        TOUCH_UP,
        TOUCH_MOVE,

        KEY_DOWN,
        KEY_UP,
        KEY_CHAR,

        FUNCTOR,

        APP_SUSPENDED,
        APP_RESUMED,

        USER_CLOSE_REQUEST,
        APP_TERMINATE,
    };

    /// Parameter for APP_TERMINATE event
    struct AppTerminateEvent
    {
        /// Flag indicating whether termination was initiated by system (value 1) or by application (value 0).
        /// System initiates termination on some platforms:
        ///     - on android when activity is finishing
        ///     - on mac when user pressed cmd+q
        uint32 triggeredBySystem;
    };

    /// Parameter for events:
    ///     - WINDOW_FOCUS_CHANGED: window got focus (value 1) or lost focus (value 0)
    ///     - WINDOW_VISIBILITY_CHANGED: window became visible (value 1) or became hidden (value 0)
    struct WindowStateEvent
    {
        uint32 state;
    };

    /// Parameter for WINDOW_DESTROYED event
    struct WindowDestroyedEvent
    {
        /// Flag indicating whether native window was truly destroyed (value 0) or detached from DAVA::Window instance (value 1)
        /// Windows are detached only on app exit
        uint32 detached;
    };

    /// Parameter for events:
    ///     - WINDOW_CREATED
    ///     - WINDOW_SIZE_SCALE_CHANGED
    struct WindowSizeEvent
    {
        float32 width;
        float32 height;
        float32 scaleX;
        float32 scaleY;
    };

    /// Parameter for mouse events:
    ///     - MOUSE_BUTTON_DOWN
    ///     - MOUSE_BUTTON_UP
    ///     - MOUSE_MOVE
    ///     - MOUSE_WHEEL
    struct MouseEvent
    {
        uint32 button; // What button is pressed (MOUSE_BUTTON_DOWN and MOUSE_BUTTON_UP)
        uint32 clicks; // Number of button clicks (MOUSE_BUTTON_DOWN)
        float32 x; // Point where mouse click, mouse wheel occured or point where mouse is moved,
        float32 y; // if isRelative then point designate relative mouse move not absolute
        float32 scrollDeltaX; // Scroll deltas for
        float32 scrollDeltaY; //      MOUSE_WHEEL event
        bool isRelative : 1; // Mouse event occured while window cursor is in pinning mode
    };

    /// Parameter for touch events:
    ///     - TOUCH_DOWN
    ///     - TOUCH_UP
    ///     - TOUCH_MOVE
    struct TouchEvent
    {
        uint32 touchId;
        float32 x;
        float32 y;
    };

    /// Parameter for events:
    ///     - KEY_DOWN
    ///     - KEY_UP
    ///     - KEY_CHAR
    struct KeyEvent
    {
        uint32 key;
        bool isRepeated;
    };

    MainDispatcherEvent() = default;
    MainDispatcherEvent(eType type)
        : type(type)
    {
    }
    MainDispatcherEvent(Window* window)
        : window(window)
    {
    }
    MainDispatcherEvent(eType type, Window* window)
        : type(type)
        , window(window)
    {
    }

    eType type = DUMMY;
    uint64 timestamp = 0;
    Window* window = nullptr;
    Function<void()> functor;
    union
    {
        AppTerminateEvent terminateEvent;
        WindowStateEvent stateEvent;
        WindowDestroyedEvent destroyedEvent;
        WindowSizeEvent sizeEvent;
        MouseEvent mouseEvent;
        TouchEvent touchEvent;
        KeyEvent keyEvent;
    };

    static MainDispatcherEvent CreateAppTerminateEvent(bool triggeredBySystem);
    static MainDispatcherEvent CreateUserCloseRequestEvent(Window* window);

    static MainDispatcherEvent CreateWindowCreatedEvent(Window* window, float32 width, float32 height, float32 scaleX, float32 scaleY);
    static MainDispatcherEvent CreateWindowDestroyedEvent(Window* window);
    static MainDispatcherEvent CreateWindowSizeChangedEvent(Window* window, float32 width, float32 height, float32 scaleX, float32 scaleY);
    static MainDispatcherEvent CreateWindowFocusChangedEvent(Window* window, bool focusState);
    static MainDispatcherEvent CreateWindowVisibilityChangedEvent(Window* window, bool visibilityState);

    static MainDispatcherEvent CreateWindowKeyPressEvent(Window* window, eType keyEventType, uint32 key, bool isRepeated);
    static MainDispatcherEvent CreateWindowMouseClickEvent(Window* window, eType mouseClickEventType, uint32 button, float32 x, float32 y, uint32 clicks, bool isRelative);
    static MainDispatcherEvent CreateWindowMouseMoveEvent(Window* window, float32 x, float32 y, bool isRelative);
    static MainDispatcherEvent CreateWindowMouseWheelEvent(Window* window, float32 x, float32 y, float32 deltaX, float32 deltaY, bool isRelative);
    static MainDispatcherEvent CreateWindowTouchEvent(Window* window, eType touchEventType, uint32 touchId, float32 x, float32 y);
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
