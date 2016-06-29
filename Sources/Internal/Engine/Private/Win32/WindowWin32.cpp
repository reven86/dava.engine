#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Win32/WindowWin32.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/Win32/CoreWin32.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{

bool WindowWin32::windowClassRegistered = false;
const wchar_t WindowWin32::windowClassName[] = L"DAVA_WND_CLASS";

WindowWin32::WindowWin32(EngineBackend* engine_, WindowBackend* window_)
    : engine(engine_)
    , dispatcher(engine->GetDispatcher())
    , window(window_)
{
}

WindowWin32::~WindowWin32()
{
    DVASSERT(hwnd == nullptr);
}

bool WindowWin32::Create(float32 width, float32 height)
{
    if (!RegisterWindowClass())
    {
        Logger::Error("Failed to register win32 window class: %d", GetLastError());
        return false;
    }

    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);
    AdjustWindowSize(&w, &h);

    HWND handle = ::CreateWindowExW(windowExStyle,
                                    windowClassName,
                                    L"DAVA_WINDOW",
                                    windowStyle,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    w,
                                    h,
                                    nullptr,
                                    nullptr,
                                    CoreWin32::Win32AppInstance(),
                                    this);
    if (handle != nullptr)
    {
        ::ShowWindow(handle, SW_SHOWNORMAL);
        ::UpdateWindow(handle);
        return true;
    }
    else
    {
        Logger::Error("Failed to create win32 window: %d", GetLastError());
    }
    return false;
}

void WindowWin32::Resize(float32 width, float32 height)
{
    PlatformEvent e;
    e.type = PlatformEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowWin32::Close()
{
    PlatformEvent e;
    e.type = PlatformEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

void WindowWin32::RunAsyncOnUIThread(const Function<void()>& task)
{
    PlatformEvent e;
    e.type = PlatformEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowWin32::TriggerPlatformEvents()
{
    ::PostMessage(hwnd, WM_CUSTOM_MESSAGE, 0, 0);
}

void WindowWin32::DoResizeWindow(float32 width, float32 height)
{
    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);
    AdjustWindowSize(&w, &h);

    UINT flags = SWP_NOMOVE | SWP_NOZORDER;
    ::SetWindowPos(hwnd, nullptr, 0, 0, w, h, flags);
}

void WindowWin32::DoCloseWindow()
{
    ::DestroyWindow(hwnd);
}

void WindowWin32::AdjustWindowSize(int32* w, int32* h)
{
    RECT rc = { 0, 0, *w, *h };
    ::AdjustWindowRectEx(&rc, windowStyle, FALSE, windowExStyle);

    *w = rc.right - rc.left;
    *h = rc.bottom - rc.top;
}

void WindowWin32::EventHandler(const PlatformEvent& e)
{
    switch (e.type)
    {
    case PlatformEvent::RESIZE_WINDOW:
        DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case PlatformEvent::CLOSE_WINDOW:
        DoCloseWindow();
        break;
    case PlatformEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

LRESULT WindowWin32::OnSize(int resizingType, int width, int height)
{
    if (resizingType == SIZE_MINIMIZED)
    {
        isMinimized = true;
        window->PostVisibilityChanged(false);
        return 0;
    }

    if (resizingType == SIZE_RESTORED || resizingType == SIZE_MAXIMIZED)
    {
        if (isMinimized)
        {
            isMinimized = false;
            window->PostVisibilityChanged(true);
            return 0;
        }
    }

    window->PostSizeChanged(static_cast<float32>(width),
                            static_cast<float32>(height),
                            1.0f,
                            1.0f);
    return 0;
}

LRESULT WindowWin32::OnSetKillFocus(bool gotFocus)
{
    window->PostFocusChanged(gotFocus);
    return 0;
}

LRESULT WindowWin32::OnMouseMoveEvent(uint16 keyModifiers, int x, int y)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::MOUSE_MOVE;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.mmoveEvent.x = static_cast<float32>(x);
    e.mmoveEvent.y = static_cast<float32>(y);
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnMouseWheelEvent(uint16 keyModifiers, int32 delta, int x, int y)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.mwheelEvent.x = static_cast<float32>(x);
    e.mwheelEvent.y = static_cast<float32>(y);
    e.mwheelEvent.delta = delta;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnMouseClickEvent(UINT message, uint16 keyModifiers, uint16 xbutton, int x, int y)
{
    DispatcherEvent e;

    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.mclickEvent.clicks = 1;
    e.mclickEvent.x = static_cast<float32>(x);
    e.mclickEvent.y = static_cast<float32>(y);

    switch (message)
    {
    case WM_LBUTTONDOWN:
        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = 1;
        break;
    case WM_LBUTTONUP:
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = 1;
        break;
    case WM_LBUTTONDBLCLK:
        return 0;
        break;
    case WM_RBUTTONDOWN:
        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = 2;
        break;
    case WM_RBUTTONUP:
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = 2;
        break;
    case WM_RBUTTONDBLCLK:
        return 0;
        break;
    case WM_MBUTTONDOWN:
        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = 3;
        break;
    case WM_MBUTTONUP:
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = 3;
        break;
    case WM_MBUTTONDBLCLK:
        return 0;
        break;
    case WM_XBUTTONDOWN:
        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = xbutton == XBUTTON1 ? 4 : 5;
        break;
    case WM_XBUTTONUP:
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = xbutton == XBUTTON1 ? 4 : 5;
        break;
    case WM_XBUTTONDBLCLK:
        return 0;
        break;
    default:
        return 0;
    }

    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated)
{
    if ((key == VK_SHIFT && scanCode == 0x36) || isExtended)
    {
        key |= 0x100;
    }

    DispatcherEvent e;
    e.type = isPressed ? DispatcherEvent::KEY_DOWN : DispatcherEvent::KEY_UP;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnCharEvent(uint32 key, bool isRepeated)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::KEY_CHAR;
    e.window = window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnCreate()
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    window->PostWindowCreated(this,
                              static_cast<float32>(rc.right - rc.left),
                              static_cast<float32>(rc.bottom - rc.top),
                              1.0f,
                              1.0f);
    window->PostVisibilityChanged(true);
    return 0;
}

LRESULT WindowWin32::OnDestroy()
{
    if (!isMinimized)
    {
        window->PostVisibilityChanged(false);
    }
    window->PostWindowDestroyed();
    hwnd = nullptr;
    return 0;
}

LRESULT WindowWin32::OnCustomMessage()
{
    platformDispatcher.ProcessEvents(MakeFunction(this, &WindowWin32::EventHandler));
    return 0;
}

LRESULT WindowWin32::WindowProc(UINT message, WPARAM wparam, LPARAM lparam, bool& isHandled)
{
    // Intentionally use 'if' instead of 'switch'
    LRESULT lresult = 0;
    if (message == WM_SIZE)
    {
        int w = GET_X_LPARAM(lparam);
        int h = GET_Y_LPARAM(lparam);
        lresult = OnSize(static_cast<int>(wparam), w, h);
    }
    else if (message == WM_ERASEBKGND)
    {
        lresult = 1;
    }
    else if (message == WM_GETMINMAXINFO)
    {
    }
    else if (message == WM_SETFOCUS || message == WM_KILLFOCUS)
    {
        lresult = OnSetKillFocus(message == WM_SETFOCUS);
    }
    else if (message == WM_ENTERSIZEMOVE || message == WM_EXITSIZEMOVE)
    {
    }
    else if (message == WM_MOUSEMOVE)
    {
        uint16 keyModifiers = GET_KEYSTATE_WPARAM(wparam);
        int x = GET_X_LPARAM(lparam);
        int y = GET_Y_LPARAM(lparam);
        lresult = OnMouseMoveEvent(keyModifiers, x, y);
    }
    else if (message == WM_MOUSEWHEEL)
    {
        int32 delta = GET_WHEEL_DELTA_WPARAM(wparam);
        uint16 keyModifiers = GET_KEYSTATE_WPARAM(wparam);
        int x = GET_X_LPARAM(lparam);
        int y = GET_Y_LPARAM(lparam);
        lresult = OnMouseWheelEvent(keyModifiers, delta, x, y);
    }
    else if (WM_MOUSEFIRST <= message && message <= WM_MOUSELAST)
    {
        uint16 keyModifiers = GET_KEYSTATE_WPARAM(wparam);
        uint16 xbutton = GET_XBUTTON_WPARAM(wparam);
        int x = GET_X_LPARAM(lparam);
        int y = GET_Y_LPARAM(lparam);
        lresult = OnMouseClickEvent(message, keyModifiers, xbutton, x, y);
    }
    else if (message == WM_KEYUP || message == WM_KEYDOWN || message == WM_SYSKEYUP || message == WM_SYSKEYDOWN)
    {
        uint32 key = static_cast<uint32>(wparam);
        uint32 scanCode = (static_cast<uint32>(lparam) << 16) & 0xFF;
        bool isPressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
        bool isExtended = (HIWORD(lparam) & KF_EXTENDED) == KF_EXTENDED;
        bool isRepeated = (HIWORD(lparam) & KF_REPEAT) == KF_REPEAT;
        lresult = OnKeyEvent(key, scanCode, isPressed, isExtended, isRepeated);
    }
    else if (message == WM_CHAR)
    {
        uint32 key = static_cast<uint32>(wparam);
        bool isRepeated = (HIWORD(lparam) & KF_REPEAT) == KF_REPEAT;
        lresult = OnCharEvent(key, isRepeated);
    }
    else if (message == WM_CUSTOM_MESSAGE)
    {
        lresult = OnCustomMessage();
    }
    else if (message == WM_CREATE)
    {
        lresult = OnCreate();
    }
    else if (message == WM_DESTROY)
    {
        lresult = OnDestroy();
    }
    else
    {
        isHandled = false;
    }
    return lresult;
}

LRESULT CALLBACK WindowWin32::WndProcStart(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    bool isHandled = true;
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        WindowWin32* pthis = static_cast<WindowWin32*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, 0, reinterpret_cast<LONG_PTR>(pthis));
        pthis->hwnd = hwnd;
    }

    // NOTE: first message coming to wndproc is not always WM_NCCREATE
    // It can be e.g. WM_GETMINMAXINFO, so do not handle all messages before WM_NCCREATE
    LRESULT lresult = 0;
    WindowWin32* pthis = reinterpret_cast<WindowWin32*>(GetWindowLongPtrW(hwnd, 0));
    if (pthis != nullptr)
    {
        lresult = pthis->WindowProc(message, wparam, lparam, isHandled);
        if (message == WM_NCDESTROY)
        {
            isHandled = false;
        }
    }
    else
    {
        isHandled = false;
    }

    if (!isHandled)
    {
        return ::DefWindowProc(hwnd, message, wparam, lparam);
    }
    return lresult;
}

bool WindowWin32::RegisterWindowClass()
{
    if (!windowClassRegistered)
    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = &WndProcStart;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(void*); // Reserve room to store 'this' pointer
        wcex.hInstance = CoreWin32::Win32AppInstance();
        wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = windowClassName;
        wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        windowClassRegistered = ::RegisterClassExW(&wcex) != 0;
    }
    return windowClassRegistered;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
