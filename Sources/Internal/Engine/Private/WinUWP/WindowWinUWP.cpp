#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/WindowWinUWP.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/WinUWP/CoreWinUWP.h"
#include "Engine/Private/WinUWP/WindowWinUWPBridge.h"

namespace DAVA
{
namespace Private
{
WindowWinUWP::WindowWinUWP(EngineBackend* e, Window* w)
    : engine(e)
    , dispatcher(engine->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowWinUWP::PlatformEventHandler))
    , bridge(ref new WindowWinUWPBridge(this))
{
}

WindowWinUWP::~WindowWinUWP() = default;

void* WindowWinUWP::GetHandle() const
{
    return bridge->GetHandle();
}

void WindowWinUWP::Resize(float32 width, float32 height)
{
    PlatformEvent e;
    e.type = PlatformEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowWinUWP::Close()
{
    PlatformEvent e;
    e.type = PlatformEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

void WindowWinUWP::RunAsyncOnUIThread(const Function<void()>& task)
{
    PlatformEvent e;
    e.type = PlatformEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowWinUWP::TriggerPlatformEvents()
{
    bridge->TriggerPlatformEvents();
}

void WindowWinUWP::ProcessPlatformEvents()
{
    // Method executes in context of XAML::Window's UI thread
    platformDispatcher.ProcessEvents();
}

void WindowWinUWP::BindXamlWindow(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    // Method executes in context of XAML::Window's UI thread
    bridge->BindToXamlWindow(xamlWindow);
}

void WindowWinUWP::PlatformEventHandler(const PlatformEvent& e)
{
    // Method executes in context of XAML::Window's UI thread
    switch (e.type)
    {
    case PlatformEvent::RESIZE_WINDOW:
        bridge->DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case PlatformEvent::CLOSE_WINDOW:
        bridge->DoCloseWindow();
        break;
    case PlatformEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
