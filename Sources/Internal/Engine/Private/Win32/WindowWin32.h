#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Engine/Private/EngineFwd.h"

#if defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace DAVA
{
class Window;

namespace Private
{
class WindowWin32 final
{
    friend class Window;

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
    LRESULT OnCreate();
    LRESULT OnDestroy();
    LRESULT WindowProc(UINT message, WPARAM wparam, LPARAM lparam, bool& isHandled);
    static LRESULT CALLBACK WndProcStart(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static bool RegisterWindowClass();

    bool CreateNativeWindow();

private:
    HWND hwnd = nullptr;
    Dispatcher* dispatcher = nullptr;
    Window* window = nullptr;

    bool isMinimized = false;

    static bool windowClassRegistered;
    static const wchar_t windowClassName[];
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
