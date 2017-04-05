#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Functional/Function.h"

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"
#include "Engine/EngineTypes.h"

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
    WindowBackend(EngineBackend* engineBackend, Window* window);
    ~WindowBackend();

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close(bool appIsTerminating);
    void SetTitle(const String& title);
    void SetMinimumSize(Size2f size);
    void SetFullscreen(eFullscreen newMode);

    void RunAsyncOnUIThread(const Function<void()>& task);
    void RunAndWaitOnUIThread(const Function<void()>& task);

    void* GetHandle() const;

    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

    void SetSurfaceScaleAsync(const float32 scale);

    void SetCursorCapture(eCursorCapture mode);
    void SetCursorVisibility(bool visible);

    void UIEventHandler(const UIDispatcherEvent& e);
    void WindowWillClose();

    EngineBackend* engineBackend = nullptr;
    Window* window = nullptr; // Window frontend reference
    MainDispatcher* mainDispatcher = nullptr; // Dispatcher that dispatches events to DAVA main thread
    UIDispatcher uiDispatcher; // Dispatcher that dispatches events to window UI thread

    std::unique_ptr<WindowNativeBridge> bridge;

    bool isMinimized = false;
    bool closeRequestByApp = false;

    // Friends
    friend class PlatformCore;
    friend struct WindowNativeBridge;
};

inline void WindowBackend::InitCustomRenderParams(rhi::InitParam& /*params*/)
{
    // No custom render params
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
