#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
struct EventWin32 final
{
    enum eType : int32
    {
        DUMMY,
        RESIZE,
        FUNCTOR
    };

    struct ResizeEvent
    {
        int32 width;
        int32 height;
    };

    eType type = DUMMY;
    Function<void()> functor;
    union
    {
        ResizeEvent resizeEvent;
    };
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
