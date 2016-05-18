#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Window.h"

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/Win32/CoreWin32.h"
#include "Engine/Private/Win32/WindowWin32.h"

#include "Logger/Logger.h"

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
}

void* WindowWin32::Handle() const
{
    return static_cast<void*>(hwnd);
}

void WindowWin32::RunAsyncOnUIThread(const Function<void()>& task)
{
}

LRESULT WindowWin32::OnSize(int resizingType, int width, int height)
{
    DispatcherEvent e;
    e.window = window;

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

    e.type = DispatcherEvent::WINDOW_SIZE_CHANGED;
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
    e.window = window;
    e.stateEvent.state = gotFocus;
    dispatcher->PostEvent(e);
    return 0;
}

LRESULT WindowWin32::OnCreate()
{
    return 0;
}

LRESULT WindowWin32::OnDestroy()
{
    DispatcherEvent e;
    e.type = DispatcherEvent::WINDOW_CLOSED;
    e.window = window;
    dispatcher->PostEvent(e);
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
    hwnd = ::CreateWindowExW(0,
                             windowClassName,
                             L"DAVA_WINDOW",
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             nullptr,
                             nullptr,
                             CoreWin32::Win32AppInstance(),
                             this);
    if (hwnd != nullptr)
    {
        ::ShowWindow(hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(hwnd);
        return true;
    }
    return false;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
