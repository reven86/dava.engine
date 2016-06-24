#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Private/EngineFwd.h"
#include "Engine/Private/Dispatcher/PlatformDispatcher.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
class WindowWin32 final
{
public:
    WindowWin32(EngineBackend* engine_, WindowBackend* window_);
    ~WindowWin32();

    WindowWin32(const WindowWin32&) = delete;
    WindowWin32& operator=(const WindowWin32&) = delete;

    void* GetHandle() const;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close();

    void RunAsyncOnUIThread(const Function<void()>& task);

    void TriggerPlatformEvents();

private:
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void AdjustWindowSize(int32* w, int32* h);

    void EventHandler(const PlatformEvent& e);

    LRESULT OnSize(int resizingType, int width, int height);
    LRESULT OnSetKillFocus(bool gotFocus);
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
    Dispatcher* dispatcher = nullptr;
    WindowBackend* window = nullptr;

    bool isMinimized = false;

    PlatformDispatcher platformDispatcher;

    static bool windowClassRegistered;
    static const wchar_t windowClassName[];
    static const UINT WM_CUSTOM_MESSAGE = WM_USER + 39;
    static const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    static const DWORD windowExStyle = 0;
};

inline void* WindowWin32::GetHandle() const
{
    return static_cast<void*>(hwnd);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
