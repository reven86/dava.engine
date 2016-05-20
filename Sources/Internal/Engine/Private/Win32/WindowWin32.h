#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Concurrency/Mutex.h"

#include "Engine/Private/EngineFwd.h"

#if defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Private/Win32/EventWin32.h"

namespace DAVA
{
class Window;

namespace Private
{
class WindowWin32 final
{
public:
    static WindowWin32* Create(Window* w);
    static void Destroy(WindowWin32* nativeWindow);

    void Resize(float32 width, float32 height);
    void* Handle() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

private:
    WindowWin32(Window* w);
    ~WindowWin32();

    WindowWin32(const WindowWin32&) = delete;
    WindowWin32& operator=(const WindowWin32&) = delete;

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

    bool CreateNativeWindow();
    void ResizeNativeWindow(int32 width, int32 height);

    void PostCustomMessage(const EventWin32& e);
    void ProcessCustomEvents();

private:
    HWND hwnd = nullptr;
    Dispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    bool isMinimized = false;

    Mutex mutex;
    Vector<EventWin32> events;

    static bool windowClassRegistered;
    static const wchar_t windowClassName[];
    static const UINT WM_CUSTOM_MESSAGE = WM_USER + 39;
    static const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    static const DWORD windowExStyle = 0;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
