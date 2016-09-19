#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WindowBackendBase.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowBackendBase::WindowBackendBase(Window& window,
                                     MainDispatcher& mainDispatcher,
                                     const Function<void(const UIDispatcherEvent&)>& uiEventHandler)
    : window(window)
    , mainDispatcher(mainDispatcher)
    , uiDispatcher(uiEventHandler)
{
}

void WindowBackendBase::RunAsyncOnUIThread(const Function<void()>& task)
{
    UIDispatcherEvent e(UIDispatcherEvent::FUNCTOR);
    e.functor = task;
    uiDispatcher.PostEvent(e);
}

void WindowBackendBase::PostResize(float32 width, float32 height)
{
    UIDispatcherEvent e(UIDispatcherEvent::RESIZE_WINDOW);
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    uiDispatcher.PostEvent(e);
}

void WindowBackendBase::PostClose(bool detach)
{
    UIDispatcherEvent e(UIDispatcherEvent::CLOSE_WINDOW);
    e.closeEvent.detach = detach;
    uiDispatcher.PostEvent(e);
}

void WindowBackendBase::PostWindowCreated(float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    MainDispatcherEvent e(MainDispatcherEvent::WINDOW_CREATED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.sizeEvent.width = width;
    e.sizeEvent.height = height;
    e.sizeEvent.scaleX = scaleX;
    e.sizeEvent.scaleY = scaleY;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostWindowCloseRequest()
{
    MainDispatcherEvent e(MainDispatcherEvent::WINDOW_CLOSE_REQUEST, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::DispatchWindowDestroyed(bool blocking)
{
    MainDispatcherEvent e(MainDispatcherEvent::WINDOW_DESTROYED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    blocking ? mainDispatcher.SendEvent(e) : mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostFocusChanged(bool focusState)
{
    MainDispatcherEvent e(MainDispatcherEvent::WINDOW_FOCUS_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.stateEvent.state = focusState;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostVisibilityChanged(bool visibilityState)
{
    MainDispatcherEvent e(MainDispatcherEvent::WINDOW_VISIBILITY_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.stateEvent.state = visibilityState;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostSizeChanged(float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    MainDispatcherEvent e(MainDispatcherEvent::WINDOW_SIZE_SCALE_CHANGED, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.sizeEvent.width = width;
    e.sizeEvent.height = height;
    e.sizeEvent.scaleX = scaleX;
    e.sizeEvent.scaleY = scaleY;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostKeyDown(uint32 key, bool isRepeated)
{
    MainDispatcherEvent e(MainDispatcherEvent::KEY_DOWN, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostKeyUp(uint32 key)
{
    MainDispatcherEvent e(MainDispatcherEvent::KEY_UP, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = false;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostKeyChar(uint32 key, bool isRepeated)
{
    MainDispatcherEvent e(MainDispatcherEvent::KEY_CHAR, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostMouseMove(float32 x, float32 y)
{
    MainDispatcherEvent e(MainDispatcherEvent::MOUSE_MOVE, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.mmoveEvent.x = x;
    e.mmoveEvent.y = y;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostMouseWheel(float32 x, float32 y, float32 deltaX, float32 deltaY)
{
    MainDispatcherEvent e(MainDispatcherEvent::MOUSE_WHEEL, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.mwheelEvent.x = x;
    e.mwheelEvent.y = y;
    e.mwheelEvent.deltaX = deltaX;
    e.mwheelEvent.deltaY = deltaY;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostTouchDown(uint32 touchId, float32 x, float32 y)
{
    MainDispatcherEvent e(MainDispatcherEvent::TOUCH_DOWN, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.tclickEvent.touchId = touchId;
    e.tclickEvent.x = x;
    e.tclickEvent.y = y;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostTouchUp(uint32 touchId, float32 x, float32 y)
{
    MainDispatcherEvent e(MainDispatcherEvent::TOUCH_UP, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.tclickEvent.touchId = touchId;
    e.tclickEvent.x = x;
    e.tclickEvent.y = y;
    mainDispatcher.PostEvent(e);
}

void WindowBackendBase::PostTouchMove(uint32 touchId, float32 x, float32 y)
{
    MainDispatcherEvent e(MainDispatcherEvent::TOUCH_MOVE, window);
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.tmoveEvent.touchId = touchId;
    e.tmoveEvent.x = x;
    e.tmoveEvent.y = y;
    mainDispatcher.PostEvent(e);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
