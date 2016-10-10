#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Dispatcher/UIDispatcherEvent.h"

namespace DAVA
{
namespace Private
{
UIDispatcherEvent UIDispatcherEvent::CreateResizeEvent(float32 width, float32 height)
{
    UIDispatcherEvent e(RESIZE_WINDOW);
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    return e;
}

UIDispatcherEvent UIDispatcherEvent::CreateCloseEvent()
{
    UIDispatcherEvent e(CLOSE_WINDOW);
    return e;
}

UIDispatcherEvent UIDispatcherEvent::CreateSetTitleEvent(const String& title)
{
    char8* buf = new char8[title.length() + 1];
    std::copy(begin(title), end(title), buf);
    buf[title.length()] = '\0';

    UIDispatcherEvent e(SET_TITLE);
    e.setTitleEvent.title = buf;
    return e;
}

UIDispatcherEvent UIDispatcherEvent::CreateFunctorEvent(const Function<void()>& functor)
{
    UIDispatcherEvent e(FUNCTOR);
    e.functor = functor;
    return e;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
