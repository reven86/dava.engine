#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Window.h"

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/Win32/CoreWin32.h"
#include "Engine/Private/Win32/WindowWin32.h"

#include "Concurrency/LockGuard.h"
#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

#if defined(__DAVAENGINE_WIN32__)

namespace DAVA
{
namespace Private
{
namespace
{
Vector<std::pair<HWND, WindowWin32*>>* hwndToThisMapping = nullptr;

WindowWin32* ThisFromHWND(HWND hwnd)
{
    if (hwndToThisMapping != nullptr)
    {
        auto it = std::find_if(hwndToThisMapping->cbegin(), hwndToThisMapping->cend(), [hwnd](const std::pair<HWND, WindowWin32*>& o) -> bool {
            return hwnd == o.first;
        });
        return it->second;
    }
    return nullptr;
}

void CreateMappingHWNDtoThis(WindowWin32* pthis, HWND hwnd)
{
    if (hwndToThisMapping == nullptr)
    {
        hwndToThisMapping = new Vector<std::pair<HWND, WindowWin32*>>;
    }

    hwndToThisMapping->emplace_back(hwnd, pthis);
}

void DeleteMappingHWNDtoThis(WindowWin32* w, HWND hwnd)
{
    auto it = std::find_if(hwndToThisMapping->cbegin(), hwndToThisMapping->cend(), [hwnd](const std::pair<HWND, WindowWin32*>& o) -> bool {
        return hwnd == o.first;
    });
    //DVASSERT(it != hwndToThisMapping->cend());
    hwndToThisMapping->erase(it);

    if (hwndToThisMapping->empty())
    {
        delete hwndToThisMapping;
        hwndToThisMapping = nullptr;
    }
}

} // anonimous namespace

bool WindowWin32::windowClassRegistered = false;
const wchar_t WindowWin32::windowClassName[] = L"DAVA_WND_CLASS";

WindowWin32* WindowWin32::Create(Window* w)
{
    WindowWin32* nativeWindow = new WindowWin32(w);
    if (!nativeWindow->CreateNativeWindow())
    {
        delete nativeWindow;
        nativeWindow = nullptr;
    }
    w->BindNativeWindow(nativeWindow);
    return nativeWindow;
}

void WindowWin32::Destroy(WindowWin32* nativeWindow)
{
}

WindowWin32::WindowWin32(Window* w)
    : dispatcher(EngineBackend::instance->dispatcher)
    , window(w)
{
    RegisterWindowClass();
}

WindowWin32::~WindowWin32()
{
    //DVASSERT(hwnd == nullptr);
}

void WindowWin32::Resize(float32 width, float32 height)
{
    EventWin32 e;
    e.type = EventWin32::RESIZE;
    e.resizeEvent.width = static_cast<int32>(width);
    e.resizeEvent.height = static_cast<int32>(height);
    PostCustomMessage(e);
}

void* WindowWin32::Handle() const
{
    return static_cast<void*>(hwnd);
}

void WindowWin32::RunAsyncOnUIThread(const Function<void()>& task)
{
    EventWin32 e;
    e.type = EventWin32::FUNCTOR;
    e.functor = task;
    PostCustomMessage(e);
}

LRESULT WindowWin32::OnSize(int resizingType, int width, int height)
{
    DispatcherEvent e;
    e.window = window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    if (resizingType == SIZE_MINIMIZED)
    {
        isMinimized = true;
        e.type = DispatcherEvent::WINDOW_VISIBILITY_CHANGED;
        e.stateEvent.state = 0;
        dispatcher->PostEvent(e);
        return 0;
    }

    if (resizingType == SIZE_RESTORED || resizingType == SIZE_MAXIMIZED)
    {
        if (isMinimized)
        {
            isMinimized = false;
            e.type = DispatcherEvent::WINDOW_VISIBILITY_CHANGED;
            e.stateEvent.state = 1;
            dispatcher->PostEvent(e);
        }
    }

    e.type = DispatcherEvent::WINDOW_SIZE_SCALE_CHANGED;
    e.sizeEvent.width = static_cast<float32>(width);
    e.sizeEvent.height = static_cast<float32>(height);
    e.sizeEvent.scaleX = 1.0f;
    e.sizeEvent.scaleY = 1.0f;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnSetKillFocus(bool gotFocus)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::WINDOW_FOCUS_CHANGED;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;
    e.stateEvent.state = gotFocus;
    dispatcher->PostEvent(e);
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
    DispatcherEvent e;

    e.type = isPressed ? DispatcherEvent::KEY_DOWN : DispatcherEvent::KEY_UP;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window;

    if ((key == VK_SHIFT && scanCode == 0x36) || isExtended)
    {
        key |= 0x100;
    }

    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;

    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnCharEvent(uint32 key, bool isRepeated)
{
    DispatcherEvent e;

    e.type = DispatcherEvent::KEY_CHAR;
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

    DispatcherEvent e;
    e.window = window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();

    e.type = DispatcherEvent::WINDOW_CREATED;
    e.sizeEvent.width = static_cast<float32>(rc.right - rc.left);
    e.sizeEvent.height = static_cast<float32>(rc.bottom - rc.top);
    e.sizeEvent.scaleX = 1.0f;
    e.sizeEvent.scaleY = 1.0f;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnDestroy()
{
    DispatcherEvent e;
    e.type = DispatcherEvent::WINDOW_DESTROYED;
    e.window = window;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnCustomMessage()
{
    ProcessCustomEvents();
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
        //Logger::Error("WM_SIZE: type=%d, w=%d, h=%d", int(wparam), w, h);
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
        CreateMappingHWNDtoThis(pthis, hwnd);
        pthis->hwnd = hwnd;
    }

    // NOTE: first message coming to wndproc is not always WM_NCCREATE
    // It can be e.g. WM_GETMINMAXINFO, so do not handle all messages before WM_NCCREATE
    LRESULT lresult = 0;
    WindowWin32* pthis = ThisFromHWND(hwnd);
    if (pthis != nullptr)
    {
        lresult = pthis->WindowProc(message, wparam, lparam, isHandled);
        if (message == WM_NCDESTROY)
        {
            DeleteMappingHWNDtoThis(pthis, hwnd);
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
        wcex.cbWndExtra = 0;
        wcex.hInstance = CoreWin32::Win32AppInstance();
        wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(HOLLOW_BRUSH));
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = windowClassName;
        wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        windowClassRegistered = ::RegisterClassExW(&wcex) != 0;
    }
    return windowClassRegistered;
}

bool WindowWin32::CreateNativeWindow()
{
    HWND hwndCheck = ::CreateWindowExW(windowExStyle,
                                       windowClassName,
                                       L"DAVA_WINDOW",
                                       windowStyle,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       nullptr,
                                       nullptr,
                                       CoreWin32::Win32AppInstance(),
                                       this);
    if (hwndCheck != nullptr)
    {
        ::ShowWindow(hwndCheck, SW_SHOWNORMAL);
        ::UpdateWindow(hwndCheck);
        return true;
    }
    return false;
}

void WindowWin32::ResizeNativeWindow(int32 width, int32 height)
{
    RECT rc = {
        0, 0,
        width,
        height
    };
    ::AdjustWindowRectEx(&rc, windowStyle, FALSE, windowExStyle);

    width = rc.right - rc.left;
    height = rc.bottom - rc.top;
    UINT flags = SWP_NOMOVE | SWP_NOZORDER;
    ::SetWindowPos(hwnd, nullptr, 0, 0, width, height, flags);
}

void WindowWin32::PostCustomMessage(const EventWin32& e)
{
    {
        LockGuard<Mutex> lock(mutex);
        events.push_back(e);
    }
    ::PostMessage(hwnd, WM_CUSTOM_MESSAGE, 0, 0);
}

void WindowWin32::ProcessCustomEvents()
{
    Vector<EventWin32> readyEvents;
    {
        LockGuard<Mutex> lock(mutex);
        events.swap(readyEvents);
    }

    EventWin32 pendingEvent;
    for (const EventWin32& e : readyEvents)
    {
        if (e.type == EventWin32::RESIZE)
        {
            pendingEvent.type = e.type;
            pendingEvent.resizeEvent = e.resizeEvent;
        }
        else if (e.type == EventWin32::FUNCTOR)
        {
            e.functor();
        }
    }

    if (pendingEvent.type == EventWin32::RESIZE)
    {
        ResizeNativeWindow(pendingEvent.resizeEvent.width, pendingEvent.resizeEvent.height);
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
