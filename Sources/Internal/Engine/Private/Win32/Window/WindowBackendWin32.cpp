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

WindowBackend::WindowBackend(EngineBackend* e, Window* w)
    : engine(e)
    , dispatcher(engine->GetDispatcher())
    , window(w)
    , platformDispatcher(MakeFunction(this, &WindowBackend::EventHandler))
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
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::RESIZE_WINDOW;
    e.resizeEvent.width = width;
    e.resizeEvent.height = height;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::Close()
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CLOSE_WINDOW;
    platformDispatcher.PostEvent(e);
}

bool WindowBackend::IsWindowReadyForRender() const
{
    return GetHandle() != nullptr;
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::FUNCTOR;
    e.functor = task;
    platformDispatcher.PostEvent(e);
}

void WindowBackend::TriggerPlatformEvents()
{
    ::PostMessage(hwnd, WM_CUSTOM_MESSAGE, 0, 0);
}

bool WindowBackend::SetCaptureMode(eCaptureMode mode)
{
    if (eCaptureMode::FRAME == mode)
    {
        //for now, not supported
        return false;
    }
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_CAPTURE_MODE;
    e.mouseMode = mode;
    platformDispatcher.PostEvent(e);
    return true;
}

bool WindowBackend::SetMouseVisibility(bool visible)
{
    UIDispatcherEvent e;
    e.type = UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY;
    e.mouseVisible = visible;
    platformDispatcher.PostEvent(e);
    return true;
}

void WindowBackend::SetCursorInCenter()
{
    RECT clientRect;
    ::GetClientRect(hwnd, &clientRect);
    int clientCenterX = static_cast<int>((clientRect.left + clientRect.right) >> 1);
    int clientCenterY = static_cast<int>((clientRect.bottom + clientRect.top) >> 1);
    POINT point;
    point.x = clientCenterX;
    point.y = clientCenterY;
    ::ClientToScreen(hwnd, &point);
    ::SetCursorPos(point.x, point.y);
    ::SetCursor(NULL);
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

void WindowBackend::DoSetCaptureMode(eCaptureMode mode)
{
    if (captureMode == mode)
    {
        return;
    }
    captureMode = mode;
    switch (mode)
    {
    case eCaptureMode::FRAME:
        //not implemented
        break;
    case eCaptureMode::PINNING:
    {
        DoSetMouseVisibility(false);
        POINT p;
        ::GetCursorPos(&p);
        lastCursorPosition.x = p.x;
        lastCursorPosition.y = p.y;
        SetCursorInCenter();
        break;
    }
    case eCaptureMode::DEFAULT:
    {
        DoSetMouseVisibility(true);
        ::SetCursorPos(lastCursorPosition.x, lastCursorPosition.y);
        break;
    }
    }
}

void WindowBackend::DoSetMouseVisibility(bool visible)
{
    if (mouseVisible == visible)
    {
        return;
    }
    mouseVisible = visible;
    if (visible)
    {
        HCURSOR defaultCursor = LoadCursor(NULL, IDC_ARROW);
        SetClassLongPtr(hwnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(defaultCursor)));
        ::SetCursor(defaultCursor);
    }
    else
    {
        SetClassLongPtr(hwnd, GCLP_HCURSOR, NULL);
        ::SetCursor(NULL);
    }
}

void WindowBackend::AdjustWindowSize(int32* w, int32* h)
{
    RECT rc = { 0, 0, *w, *h };
    ::AdjustWindowRectEx(&rc, windowStyle, FALSE, windowExStyle);

    *w = rc.right - rc.left;
    *h = rc.bottom - rc.top;
}

void WindowBackend::EventHandler(const UIDispatcherEvent& e)
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
    case UIDispatcherEvent::CHANGE_CAPTURE_MODE:
        DoSetCaptureMode(e.mouseMode);
        break;
    case UIDispatcherEvent::CHANGE_MOUSE_VISIBILITY:
        DoSetMouseVisibility(e.mouseVisible);
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

LRESULT WindowBackend::OnSetKillFocus(bool gotFocus)
{
    window->PostFocusChanged(gotFocus);
    return 0;
}

LRESULT WindowBackend::OnMouseHoverEvent()
{
    mouseTracking = false; // tracking now cancelled
    // set up leave tracking
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = hwnd;
    mouseTracking = (::TrackMouseEvent(&tme) != 0);

    ::SetCursor(NULL);
    return 0;
}

LRESULT WindowBackend::OnMouseLeaveEvent()
{
    mouseTracking = false; // tracking now cancelled
    return 0;
}

LRESULT WindowBackend::OnMouseMoveEvent(uint16 keyModifiers, int x, int y)
{
    if (!mouseTracking && !mouseVisible)
    {
        // start tracking if we aren't already
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = hwnd;
        tme.dwHoverTime = HOVER_DEFAULT;
        mouseTracking = (::TrackMouseEvent(&tme) != 0);
    }

    if (eCaptureMode::PINNING == captureMode)
    {
        RECT clientRect;
        ::GetClientRect(hwnd, &clientRect);
        int clientCenterX = static_cast<int>((clientRect.left + clientRect.right) >> 1);
        int clientCenterY = static_cast<int>((clientRect.bottom + clientRect.top) >> 1);
        int shiftX = x - clientCenterX;
        int shiftY = y - clientCenterY;
        if (shiftX != 0 || shiftY != 0)
        {
            SetCursorInCenter();
            x = shiftX;
            y = shiftY;
        }
        else
        {
            // skip mouse moveEvent, which generate SetCursorPos
            return 0;
        }
    }
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::MOUSE_MOVE;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.mmoveEvent.x = static_cast<float32>(x);
    e.mmoveEvent.y = static_cast<float32>(y);
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowBackend::OnMouseWheelEvent(uint16 keyModifiers, int32 delta, int x, int y)
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.mwheelEvent.x = static_cast<float32>(x);
    e.mwheelEvent.y = static_cast<float32>(y);
    e.mwheelEvent.deltaX = 0.0f;
    e.mwheelEvent.deltaY = static_cast<float32>(delta);
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowBackend::OnMouseClickEvent(UINT message, uint16 keyModifiers, uint16 xbutton, int x, int y)
{
    MainDispatcherEvent e;

    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
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
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowBackend::OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated)
{
    if ((key == VK_SHIFT && scanCode == 0x36) || isExtended)
    {
        key |= 0x100;
    }

    MainDispatcherEvent e;
    e.type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowBackend::OnCharEvent(uint32 key, bool isRepeated)
{
    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::KEY_CHAR;
    e.window = window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowBackend::OnCreate()
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

LRESULT WindowBackend::OnDestroy()
{
    if (!isMinimized)
    {
        window->PostVisibilityChanged(false);
    }
    window->PostWindowDestroyed();
    hwnd = nullptr;
    return 0;
}

LRESULT WindowBackend::OnCustomMessage()
{
    platformDispatcher.ProcessEvents();
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
    else if (message == WM_MOUSEHOVER)
    {
        lresult = OnMouseHoverEvent();
    }
    else if (message == WM_MOUSELEAVE)
    {
        lresult = OnMouseLeaveEvent();
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
