#if defined(__DAVAENGINE_COREV2__)

#pragma once

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

#endif // __DAVAENGINE_COREV2__
