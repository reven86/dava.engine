#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"
#include "Engine/Public/EngineTypes.h"

#include "Functional/Function.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final
{
public:
    WindowBackend(EngineBackend* e, Window* w);
    ~WindowBackend();

    void* GetHandle() const;
    MainDispatcher* GetDispatcher() const;
    Window* GetWindow() const;
    WindowNativeService* GetNativeService() const;

    void Resize(float32 width, float32 height);
    void Close();
    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

    void BindXamlWindow(::Windows::UI::Xaml::Window ^ xamlWindow);

    void SetMouseMode(eMouseMode mode);
    eMouseMode GetMouseMode() const;

private:
    void PlatformEventHandler(const UIDispatcherEvent& e);

private:
    EngineBackend* engine = nullptr;
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    UIDispatcher platformDispatcher;

    ref struct WindowNativeBridge ^ bridge = nullptr;
    std::unique_ptr<WindowNativeService> nativeService;
};

inline MainDispatcher* WindowBackend::GetDispatcher() const
{
    return dispatcher;
}

inline Window* WindowBackend::GetWindow() const
{
    return window;
}

inline WindowNativeService* WindowBackend::GetNativeService() const
{
    return nativeService.get();
}

inline void WindowBackend::InitCustomRenderParams(rhi::InitParam& /*params*/)
{
    // No custom render params
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
