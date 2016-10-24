#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Win32/Window/WindowBackendWin32.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include <ShellScalingAPI.h>

#include "Engine/Window.h"
#include "Engine/Win32/WindowNativeServiceWin32.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Win32/PlatformCoreWin32.h"

#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
bool WindowBackend::windowClassRegistered = false;
const wchar_t WindowBackend::windowClassName[] = L"DAVA_WND_CLASS";

WindowBackend::WindowBackend(EngineBackend* engineBackend, Window* window)
    : engineBackend(engineBackend)
    , window(window)
    , mainDispatcher(engineBackend->GetDispatcher())
    , uiDispatcher(MakeFunction(this, &WindowBackend::UIEventHandler))
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
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateResizeEvent(width, height));
}

void WindowBackend::Close(bool /*appIsTerminating*/)
{
    closeRequestByApp = true;
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateCloseEvent());
}

void WindowBackend::SetTitle(const String& title)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateSetTitleEvent(title));
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    uiDispatcher.PostEvent(UIDispatcherEvent::CreateFunctorEvent(task));
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

void WindowBackend::DoSetTitle(const char8* title)
{
    WideString wideTitle = UTF8Utils::EncodeToWideString(title);
    ::SetWindowTextW(hwnd, wideTitle.c_str());
}

void WindowBackend::AdjustWindowSize(int32* w, int32* h)
{
    RECT rc = { 0, 0, *w, *h };
    ::AdjustWindowRectEx(&rc, windowStyle, FALSE, windowExStyle);

    *w = rc.right - rc.left;
    *h = rc.bottom - rc.top;
}

void WindowBackend::HandleSizeChanged(int32 w, int32 h)
{
    // Do not send excessive size changed events
    if (lastWidth != w || lastHeight != h)
    {
        lastWidth = w;
        lastHeight = h;

        float32 width = static_cast<float32>(lastWidth);
        float32 height = static_cast<float32>(lastHeight);

        // on win32 surfaceWidth/surfaceHeight is same as window width/height
        float32 surfaceWidth = width;
        float32 surfaceHeight = height;

        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, width, width, surfaceWidth, surfaceHeight));
    }
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
    case UIDispatcherEvent::SET_TITLE:
        DoSetTitle(e.setTitleEvent.title);
        delete[] e.setTitleEvent.title;
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
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
        return 0;
    }

    if (resizingType == SIZE_RESTORED || resizingType == SIZE_MAXIMIZED)
    {
        if (isMinimized)
        {
            isMinimized = false;
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
            return 0;
        }
    }
    if (!isEnteredSizingModalLoop)
    {
        HandleSizeChanged(width, height);
    }
    return 0;
}

LRESULT WindowBackend::OnEnterSizeMove()
{
    isEnteredSizingModalLoop = true;
    return 0;
}

LRESULT WindowBackend::OnExitSizeMove()
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);

    int32 w = rc.right - rc.left;
    int32 h = rc.bottom - rc.top;
    HandleSizeChanged(w, h);

    isEnteredSizingModalLoop = false;
    return 0;
}

LRESULT WindowBackend::OnDpiChanged()
{
    float32 curDpi = GetCurrentDpi();
    if (dpi != curDpi)
    {
        dpi = curDpi;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowDpiChangedEvent(window, dpi));
    }

    return 0;
}

LRESULT WindowBackend::OnSetKillFocus(bool hasFocus)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, hasFocus));
    return 0;
}

LRESULT WindowBackend::OnMouseMoveEvent(uint16 keyModifiers, int x, int y)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, static_cast<float32>(x), static_cast<float32>(y), false));
    return 0;
}

LRESULT WindowBackend::OnMouseWheelEvent(uint16 keyModifiers, int32 delta, int x, int y)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window,
                                                                               static_cast<float32>(x),
                                                                               static_cast<float32>(y),
                                                                               0.f,
                                                                               static_cast<float32>(delta),
                                                                               false));
    return 0;
}

LRESULT WindowBackend::OnMouseClickEvent(UINT message, uint16 keyModifiers, uint16 xbutton, int x, int y)
{
    uint32 button = 0;
    MainDispatcherEvent::eType type = MainDispatcherEvent::DUMMY;
    switch (message)
    {
    case WM_LBUTTONDOWN:
        type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        button = 1;
        break;
    case WM_LBUTTONUP:
        type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        button = 1;
        break;
    case WM_LBUTTONDBLCLK:
        // TODO: somehow handle mouse double-click
        return 0;
    case WM_RBUTTONDOWN:
        type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        button = 2;
        break;
    case WM_RBUTTONUP:
        type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        button = 2;
        break;
    case WM_RBUTTONDBLCLK:
        // TODO: somehow handle mouse double-click
        return 0;
    case WM_MBUTTONDOWN:
        type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        button = 3;
        break;
    case WM_MBUTTONUP:
        type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        button = 3;
        break;
    case WM_MBUTTONDBLCLK:
        // TODO: somehow handle mouse double-click
        return 0;
    case WM_XBUTTONDOWN:
        type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        button = xbutton == XBUTTON1 ? 4 : 5;
        break;
    case WM_XBUTTONUP:
        type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        button = xbutton == XBUTTON1 ? 4 : 5;
        break;
    case WM_XBUTTONDBLCLK:
        // TODO: somehow handle mouse double-click
        return 0;
    default:
        return 0;
    }

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window,
                                                                               type,
                                                                               button,
                                                                               static_cast<float32>(x),
                                                                               static_cast<float32>(y),
                                                                               1,
                                                                               false));
    return 0;
}

LRESULT WindowBackend::OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated)
{
    if ((key == VK_SHIFT && scanCode == 0x36) || isExtended)
    {
        key |= 0x100;
    }

    MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, isRepeated));
    return 0;
}

LRESULT WindowBackend::OnCharEvent(uint32 key, bool isRepeated)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, key, isRepeated));
    return 0;
}

LRESULT WindowBackend::OnCreate()
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);

    lastWidth = rc.right - rc.left;
    lastHeight = rc.bottom - rc.top;

    float32 width = static_cast<float32>(lastWidth);
    float32 height = static_cast<float32>(lastHeight);
    float32 surfaceWidth = width;
    float32 surfaceHeight = height;
    float32 dpi = GetCurrentDpi();

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, width, height, surfaceWidth, surfaceHeight, dpi));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, true));
    return 0;
}

bool WindowBackend::OnClose()
{
    if (!closeRequestByApp)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateUserCloseRequestEvent(window));
    }
    return closeRequestByApp;
}

LRESULT WindowBackend::OnDestroy()
{
    if (!isMinimized)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
    }

    mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));
    hwnd = nullptr;
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
    else if (message == WM_ENTERSIZEMOVE)
    {
        lresult = OnEnterSizeMove();
    }
    else if (message == WM_EXITSIZEMOVE)
    {
        lresult = OnExitSizeMove();
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
        // Forward WM_SYSKEYUP and WM_SYSKEYDOWN to DefWindowProc to allow system shortcuts: Alt+F4, etc
        isHandled = (message == WM_KEYUP || message == WM_KEYDOWN);
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
    else if (message == WM_DPICHANGED)
    {
        RECT* suggestedSize = reinterpret_cast<RECT*>(lparam);
        float32 w = static_cast<float32>(suggestedSize->right - suggestedSize->left);
        float32 h = static_cast<float32>(suggestedSize->bottom - suggestedSize->top);
        Resize(w, h);

        lresult = OnDpiChanged();
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

float32 WindowBackend::GetCurrentDpi() const
{
    float32 ret = 0.0f;

    using MonitorDpiFn = HRESULT(WINAPI*)(_In_ HMONITOR, _In_ MONITOR_DPI_TYPE, _Out_ UINT*, _Out_ UINT*);

    // we are trying to get pointer on GetDpiForMonitor function with GetProcAddress
    // because this function is available only on win8.1 and win10 but we should be able
    // to run the same build on win7, win8, win10. So on win7 GetProcAddress will return null
    // and GetDpiForMonitor wont be called
    HMODULE module = GetModuleHandle(TEXT("shcore.dll"));
    MonitorDpiFn fn = reinterpret_cast<MonitorDpiFn>(GetProcAddress(module, "GetDpiForMonitor"));

    if (nullptr != fn)
    {
        UINT x = 0;
        UINT y = 0;
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        (*fn)(monitor, MDT_EFFECTIVE_DPI, &x, &y);

        ret = static_cast<float32>(x);
    }
    else
    {
        // default behavior for windows (ver < 8.1)
        // get dpi from caps
        HDC screen = GetDC(NULL);
        ret = static_cast<float32>(GetDeviceCaps(screen, LOGPIXELSX));
        ReleaseDC(NULL, screen);
    }

    return ret;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
