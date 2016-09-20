#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/WindowBackendBase.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final : public WindowBackendBase
{
public:
    WindowBackend(EngineBackend* engineBackend, Window* window);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();
    void Detach();

    void* GetHandle() const;
    WindowNativeService* GetNativeService() const;

    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

private:
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void AdjustWindowSize(int32* w, int32* h);

    void UIEventHandler(const UIDispatcherEvent& e);

    LRESULT OnSize(int resizingType, int width, int height);
    LRESULT OnEnterExitSizeMove(bool enter);
    LRESULT OnSetKillFocus(bool gotFocus);
    LRESULT OnMouseMoveEvent(uint16 keyModifiers, int x, int y);
    LRESULT OnMouseWheelEvent(uint16 keyModifiers, int32 delta, int x, int y);
    LRESULT OnMouseClickEvent(UINT message, uint16 keyModifiers, uint16 xbutton, int x, int y);
    LRESULT OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated);
    LRESULT OnCharEvent(uint32 key, bool isRepeated);
    LRESULT OnCreate();
    bool OnClose();
    LRESULT OnDestroy();
    LRESULT WindowProc(UINT message, WPARAM wparam, LPARAM lparam, bool& isHandled);
    static LRESULT CALLBACK WndProcStart(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static bool RegisterWindowClass();

private:
    HWND hwnd = nullptr;
    std::unique_ptr<WindowNativeService> nativeService;

    bool isMinimized = false;
    bool isEnteredSizingModalLoop = false;
    bool closeRequestByApp = false;

    static bool windowClassRegistered;
    static const wchar_t windowClassName[];
    static const UINT WM_TRIGGER_EVENTS = WM_USER + 39;
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
