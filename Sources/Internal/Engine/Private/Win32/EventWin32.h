#if defined(__DAVAENGINE_COREV2__)

#pragma once

#if defined(__DAVAENGINE_WIN32__) && !defined(__DAVAENGINE_QT__)

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
