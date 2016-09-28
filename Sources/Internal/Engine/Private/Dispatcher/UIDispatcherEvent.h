#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Engine/EngineTypes.h"

namespace DAVA
{
namespace Private
{
struct UIDispatcherEvent final
{
    enum eType : int32
    {
        DUMMY = 0,
        RESIZE_WINDOW,
        CREATE_WINDOW,
        CLOSE_WINDOW,
        FUNCTOR,
        CHANGE_CAPTURE_MODE,
        CHANGE_MOUSE_VISIBILITY,
    };

    struct ResizeEvent
    {
        float32 width;
        float32 height;
    };

    eType type = DUMMY;
    Function<void()> functor;
    union
    {
        ResizeEvent resizeEvent;
        eCaptureMode mouseMode;
        bool mouseVisibility;
    };
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
