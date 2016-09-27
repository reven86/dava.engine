#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Functional/Function.h"

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
        SET_TITLE,
        FUNCTOR,
    };

    struct ResizeEvent
    {
        float32 width;
        float32 height;
    };

    struct SetTitleEvent
    {
        const char8* title;
    };

    UIDispatcherEvent() = default;
    UIDispatcherEvent(eType type)
        : type(type)
    {
    }

    eType type = DUMMY;
    Function<void()> functor;
    union
    {
        ResizeEvent resizeEvent;
        SetTitleEvent setTitleEvent;
    };
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
