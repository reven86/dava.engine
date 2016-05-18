#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

namespace DAVA
{
class Window;

namespace Private
{
struct DispatcherEvent final
{
    enum eType : int
    {
        WINDOW_CLOSED,
        WINDOW_FOCUS_CHANGED,
        WINDOW_VISIBILITY_CHANGED,
        WINDOW_SIZE_CHANGED,
        WINDOW_SCALE_CHANGED,
        FUNCTOR
    };

    struct WindowStateEvent
    {
        uint32 state;
    };

    struct WindowSizeEvent
    {
        float32 width;
        float32 height;
        float32 scaleX;
        float32 scaleY;
    };

    eType type;
    Window* window;
    Function<void()> functor;
    union
    {
        WindowStateEvent stateEvent;
        WindowSizeEvent sizeEvent;
    };
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
