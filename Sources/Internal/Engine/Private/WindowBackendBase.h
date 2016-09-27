#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"

#include "Functional/Function.h"

namespace DAVA
{
/// \ingroup engine_private
namespace Private
{
/// \class Private::WindowBackendBase
/// \ingroup engine_private
///
/// Private::WindowBackendBase is base class for each platform dependent Private::WindowBackend class.
/// Private::WindowBackendBase provides some useful utility methods, such as posting
/// events to main dispatcher or to UI dispatcher
class WindowBackendBase
{
protected:
    WindowBackendBase(EngineBackend& engineBackend,
                      Window& window,
                      const Function<void(const UIDispatcherEvent&)>& uiEventHandler);

    // Make all members public (except constructor) to allow seamless access from WindowNativeBridge classes
public:
    // Utility methods to dispatch events to window UI thread
    void RunAsyncOnUIThread(const Function<void()>& task);
    void PostResizeOnUIThread(float32 width, float32 height);
    void PostCloseOnUIThread();
    void PostSetTitleOnUIThread(const String& title);

    // Utility methods to dispatch events to DAVA main thread, usually from window UI thread
    void PostWindowCreated(float32 width, float32 height, float32 scaleX, float32 scaleY);
    void PostUserCloseRequest();
    void DispatchWindowDestroyed(bool blocking);

    void PostFocusChanged(bool focusState);
    void PostVisibilityChanged(bool visibilityState);
    void PostSizeChanged(float32 width, float32 height, float32 scaleX, float32 scaleY);
    void PostKeyDown(uint32 key, bool isRepeated);
    void PostKeyUp(uint32 key);
    void PostKeyChar(uint32 key, bool isRepeated);
    void PostMouseDown(uint32 button, float32 x, float32 y, uint32 clicks);
    void PostMouseUp(uint32 button, float32 x, float32 y);
    void PostMouseMove(float32 x, float32 y);
    void PostMouseWheel(float32 x, float32 y, float32 deltaX, float32 deltaY);
    void PostTouchDown(uint32 touchId, float32 x, float32 y);
    void PostTouchUp(uint32 touchId, float32 x, float32 y);
    void PostTouchMove(uint32 touchId, float32 x, float32 y);

    EngineBackend& engineBackend;
    Window& window; // Window frontend reference
    MainDispatcher& mainDispatcher; // Dispatcher that dispatches events to DAVA main thread
    UIDispatcher uiDispatcher; // Dispatcher that dispatches events to window UI thread
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
