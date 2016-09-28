#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"
#include "Engine/EngineTypes.h"

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

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    void* GetHandle() const;
    WindowNativeService* GetNativeService() const;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();
    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();

    bool SetCaptureMode(eCaptureMode mode);
    bool SetMouseVisibility(bool visible);

private:
    eCaptureMode captureMode = eCaptureMode::DEFAULT;
    bool mouseVisible = true;

    void SetCursorInCenter();
    Point2i lastCursorPosition;

    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();
    void DoSetCaptureMode(eCaptureMode mode);
    void DoSetMouseVisibility(bool visible);

    void AdjustWindowSize(int32* w, int32* h);

    void EventHandler(const UIDispatcherEvent& e);

    LRESULT OnSize(int resizingType, int width, int height);
    LRESULT OnSetKillFocus(bool gotFocus);
    LRESULT OnMouseHoverEvent();
    LRESULT OnMouseLeaveEvent();
    LRESULT OnMouseMoveEvent(uint16 keyModifiers, int x, int y);
    LRESULT OnMouseWheelEvent(uint16 keyModifiers, int32 delta, int x, int y);
    LRESULT OnMouseClickEvent(UINT message, uint16 keyModifiers, uint16 xbutton, int x, int y);
    LRESULT OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated);
    LRESULT OnCharEvent(uint32 key, bool isRepeated);
    LRESULT OnCreate();
    LRESULT OnDestroy();
    LRESULT OnCustomMessage();
    LRESULT WindowProc(UINT message, WPARAM wparam, LPARAM lparam, bool& isHandled);
    static LRESULT CALLBACK WndProcStart(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static bool RegisterWindowClass();

private:
    HWND hwnd = nullptr;
    EngineBackend* engine = nullptr;
    MainDispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    bool isMinimized = false;
    bool mouseTracking = false;

    UIDispatcher platformDispatcher;
    std::unique_ptr<WindowNativeService> nativeService;

    static bool windowClassRegistered;
    static const wchar_t windowClassName[];
    static const UINT WM_CUSTOM_MESSAGE = WM_USER + 39;
    static const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    static const DWORD windowExStyle = 0;
};

inline void* WindowBackend::GetHandle() const
{
    return static_cast<void*>(hwnd);
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

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
