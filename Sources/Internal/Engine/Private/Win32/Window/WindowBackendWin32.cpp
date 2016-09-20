#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Win32/Window/WindowBackendWin32.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Window.h"
#include "Engine/Win32/WindowNativeServiceWin32.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Win32/PlatformCoreWin32.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
bool WindowBackend::windowClassRegistered = false;
const wchar_t WindowBackend::windowClassName[] = L"DAVA_WND_CLASS";

WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : WindowBackendBase(*window,
                        *engineBackend->GetDispatcher(),
                        MakeFunction(this, &WindowBackend::UIEventHandler))
    , nativeService(new WindowNativeService(this))
{
}

WindowBackend::~WindowBackend()
{
    DVASSERT(hwnd == nullptr);
}

bool WindowBackend::Create(float32 width, float32 height)
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
                                    PlatformCore::Win32AppInstance(),
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

void WindowBackend::Resize(float32 width, float32 height)
{
    PostResize(width, height);
}

void WindowBackend::Close()
{
    closeRequestByApp = true;
    DoCloseWindow();
}

void WindowBackend::Detach()
{
    // On Win32 detach is similar to close
    Close();
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::TriggerPlatformEvents()
{
    if (uiDispatcher.HasEvents())
    {
        ::PostMessage(hwnd, WM_TRIGGER_EVENTS, 0, 0);
    }
}

void WindowBackend::ProcessPlatformEvents()
{
    uiDispatcher.ProcessEvents();
}

void WindowBackend::DoResizeWindow(float32 width, float32 height)
{
    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);
    AdjustWindowSize(&w, &h);

    UINT flags = SWP_NOMOVE | SWP_NOZORDER;
    ::SetWindowPos(hwnd, nullptr, 0, 0, w, h, flags);
}

void WindowBackend::DoCloseWindow()
{
    ::DestroyWindow(hwnd);
}

void WindowBackend::AdjustWindowSize(int32* w, int32* h)
{
    RECT rc = { 0, 0, *w, *h };
    ::AdjustWindowRectEx(&rc, windowStyle, FALSE, windowExStyle);

    *w = rc.right - rc.left;
    *h = rc.bottom - rc.top;
}

void WindowBackend::UIEventHandler(const UIDispatcherEvent& e)
{
    switch (e.type)
    {
    case UIDispatcherEvent::RESIZE_WINDOW:
        DoResizeWindow(e.resizeEvent.width, e.resizeEvent.height);
        break;
    case UIDispatcherEvent::CLOSE_WINDOW:
        DoCloseWindow();
        break;
    case UIDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    default:
        break;
    }
}

LRESULT WindowBackend::OnSize(int resizingType, int width, int height)
{
    if (resizingType == SIZE_MINIMIZED)
    {
        isMinimized = true;
        PostVisibilityChanged(false);
        return 0;
    }

    if (resizingType == SIZE_RESTORED || resizingType == SIZE_MAXIMIZED)
    {
        if (isMinimized)
        {
            isMinimized = false;
            PostVisibilityChanged(true);
            return 0;
        }
    }

    if (!isEnteredSizingModalLoop)
    {
        float32 w = static_cast<float32>(width);
        float32 h = static_cast<float32>(height);
        PostSizeChanged(w, h, 1.0f, 1.0f);
    }
    return 0;
}

LRESULT WindowBackend::OnEnterExitSizeMove(bool enter)
{
    isEnteredSizingModalLoop = enter;
    if (!enter)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        float32 w = static_cast<float32>(rc.right - rc.left);
        float32 h = static_cast<float32>(rc.bottom - rc.top);
        PostSizeChanged(w, h, 1.0f, 1.0f);
    }
    return 0;
}

LRESULT WindowBackend::OnSetKillFocus(bool gotFocus)
{
    PostFocusChanged(gotFocus);
    return 0;
}

LRESULT WindowBackend::OnMouseMoveEvent(uint16 keyModifiers, int x, int y)
{
    PostMouseMove(static_cast<float32>(x), static_cast<float32>(y));
    return 0;
}

LRESULT WindowBackend::OnMouseWheelEvent(uint16 keyModifiers, int32 delta, int x, int y)
{
    PostMouseWheel(static_cast<float32>(x), static_cast<float32>(y), 0.f, static_cast<float32>(delta));
    return 0;
}

LRESULT WindowBackend::OnMouseClickEvent(UINT message, uint16 keyModifiers, uint16 xbutton, int x, int y)
{
    MainDispatcherEvent e(window);

    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.mclickEvent.clicks = 1;
    e.mclickEvent.x = static_cast<float32>(x);
    e.mclickEvent.y = static_cast<float32>(y);

    switch (message)
    {
    case WM_LBUTTONDOWN:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = 1;
        break;
    case WM_LBUTTONUP:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = 1;
        break;
    case WM_LBUTTONDBLCLK:
        return 0;
        break;
    case WM_RBUTTONDOWN:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = 2;
        break;
    case WM_RBUTTONUP:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = 2;
        break;
    case WM_RBUTTONDBLCLK:
        return 0;
        break;
    case WM_MBUTTONDOWN:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = 3;
        break;
    case WM_MBUTTONUP:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = 3;
        break;
    case WM_MBUTTONDBLCLK:
        return 0;
        break;
    case WM_XBUTTONDOWN:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.button = xbutton == XBUTTON1 ? 4 : 5;
        break;
    case WM_XBUTTONUP:
        e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.button = xbutton == XBUTTON1 ? 4 : 5;
        break;
    case WM_XBUTTONDBLCLK:
        return 0;
        break;
    default:
        return 0;
    }

    mainDispatcher.PostEvent(e);
    return 0;
}

LRESULT WindowBackend::OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated)
{
    if ((key == VK_SHIFT && scanCode == 0x36) || isExtended)
    {
        key |= 0x100;
    }

    if (isPressed)
    {
        PostKeyDown(key, isRepeated);
    }
    else
    {
        PostKeyUp(key);
    }
    return 0;
}

LRESULT WindowBackend::OnCharEvent(uint32 key, bool isRepeated)
{
    PostKeyChar(key, isRepeated);
    return 0;
}

LRESULT WindowBackend::OnCreate()
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    float32 w = static_cast<float32>(rc.right - rc.left);
    float32 h = static_cast<float32>(rc.bottom - rc.top);
    PostWindowCreated(w, h, 1.0f, 1.0f);
    PostVisibilityChanged(true);
    return 0;
}

bool WindowBackend::OnClose()
{
    if (!closeRequestByApp)
    {
        PostUserCloseRequest();
    }
    return closeRequestByApp;
}

LRESULT WindowBackend::OnDestroy()
{
    if (!isMinimized)
    {
        PostVisibilityChanged(false);
    }
    hwnd = nullptr;
    DispatchWindowDestroyed(false);
    return 0;
}

LRESULT WindowBackend::WindowProc(UINT message, WPARAM wparam, LPARAM lparam, bool& isHandled)
{
    // Intentionally use 'if' instead of 'switch'
    LRESULT lresult = 0;
    if (message == WM_SIZE)
    {
        int w = GET_X_LPARAM(lparam);
        int h = GET_Y_LPARAM(lparam);
        lresult = OnSize(static_cast<int>(wparam), w, h);
    }
    else if (message == WM_ENTERSIZEMOVE || message == WM_EXITSIZEMOVE)
    {
        lresult = OnEnterExitSizeMove(message == WM_ENTERSIZEMOVE);
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
        int32 delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
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
    else if (message == WM_TRIGGER_EVENTS)
    {
        ProcessPlatformEvents();
    }
    else if (message == WM_CREATE)
    {
        lresult = OnCreate();
    }
    else if (message == WM_CLOSE)
    {
        isHandled = !OnClose();
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

LRESULT CALLBACK WindowBackend::WndProcStart(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    bool isHandled = true;
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
        WindowBackend* pthis = static_cast<WindowBackend*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, 0, reinterpret_cast<LONG_PTR>(pthis));
        pthis->hwnd = hwnd;
    }

    // NOTE: first message coming to wndproc is not always WM_NCCREATE
    // It can be e.g. WM_GETMINMAXINFO, so do not handle all messages before WM_NCCREATE
    LRESULT lresult = 0;
    WindowBackend* pthis = reinterpret_cast<WindowBackend*>(GetWindowLongPtrW(hwnd, 0));
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

bool WindowBackend::RegisterWindowClass()
{
    if (!windowClassRegistered)
    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = &WndProcStart;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(void*); // Reserve room to store 'this' pointer
        wcex.hInstance = PlatformCore::Win32AppInstance();
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
