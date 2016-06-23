#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"
#include "Engine/Private/Dispatcher/PlatformDispatcher.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{

class WindowWinUWP final
{
public:
    WindowWinUWP(EngineBackend* e, WindowBackend* w);
    ~WindowWinUWP();

    void* GetHandle() const;
    Dispatcher* GetDispatcher() const;
    WindowBackend* GetWindowBackend() const;

    void Resize(float32 width, float32 height);
    void Close();

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

    void BindXamlWindow(::Windows::UI::Xaml::Window^ xamlWindow);

private:
    void EventHandler(const PlatformEvent& e);

private:
    EngineBackend* engine = nullptr;
    Dispatcher* dispatcher = nullptr;
    WindowBackend* window = nullptr;

    PlatformDispatcher platformDispatcher;

    ref struct WindowWinUWPBridge ^ bridge = nullptr;

    // Friends
    friend class CoreWinUWP;
    friend ref struct WindowWinUWPBridge;
};

inline Dispatcher* WindowWinUWP::GetDispatcher() const
{
    return dispatcher;
}

inline WindowBackend* WindowWinUWP::GetWindowBackend() const
{
    return window;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
