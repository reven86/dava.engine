#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/Win32/Window/WindowBackendWin32.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include "Engine/Window.h"
#include "Engine/Win32/WindowNativeServiceWin32.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/Win32/DllImportWin32.h"
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
    if (width != w || height != h)
    {
        width = w;
        height = h;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window,
                                                                                    static_cast<float32>(width),
                                                                                    static_cast<float32>(height),
                                                                                    1.0f,
                                                                                    1.0f));
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

LRESULT WindowBackend::OnSize(int32 resizingType, int32 width, int32 height)
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

LRESULT WindowBackend::OnGetMinMaxInfo(MINMAXINFO* minMaxInfo)
{
    // Limit minimum window size to some reasonable value
    minMaxInfo->ptMinTrackSize.x = 128;
    minMaxInfo->ptMinTrackSize.y = 128;
    return 0;
}

LRESULT WindowBackend::OnSetKillFocus(bool hasFocus)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, hasFocus));
    return 0;
}

LRESULT WindowBackend::OnMouseMoveEvent(int32 x, int32 y)
{
    // Windows generates WM_MOUSEMOVE event for primary touch point so check and process
    // mouse move only from mouse device. Also skip spurious move events as described in:
    // https://blogs.msdn.microsoft.com/oldnewthing/20031001-00/?p=42343/
    eInputDevice source = GetInputEventSourceLegacy(::GetMessageExtraInfo());
    if (source == eInputDevice::MOUSE && (x != lastMouseMoveX || y != lastMouseMoveY))
    {
        eModifierKeys modifierKeys = GetModifierKeys();
        float32 vx = static_cast<float32>(x);
        float32 vy = static_cast<float32>(y);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, vx, vy, modifierKeys, false));

        lastMouseMoveX = x;
        lastMouseMoveY = y;
    }
    return 0;
}

LRESULT WindowBackend::OnMouseWheelEvent(int32 deltaX, int32 deltaY, int32 x, int32 y)
{
    eModifierKeys modifierKeys = GetModifierKeys();
    float32 vx = static_cast<float32>(x);
    float32 vy = static_cast<float32>(y);
    float32 vdeltaX = static_cast<float32>(deltaX);
    float32 vdeltaY = static_cast<float32>(deltaY);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, vx, vy, vdeltaX, vdeltaY, modifierKeys, false));
    return 0;
}

LRESULT WindowBackend::OnMouseClickEvent(UINT message, uint16 xbutton, int32 x, int32 y)
{
    // Windows generates WM_xBUTTONDONW/WM_xBUTTONUP event for primary touch point so check and process
    // mouse clicks only from mouse device.
    eInputDevice source = GetInputEventSourceLegacy(::GetMessageExtraInfo());
    if (source == eInputDevice::MOUSE)
    {
        uint32 newMouseButtonsState = mouseButtonsState;
        switch (message)
        {
        case WM_LBUTTONDOWN:
            newMouseButtonsState |= MK_LBUTTON;
            break;
        case WM_LBUTTONUP:
            newMouseButtonsState &= ~MK_LBUTTON;
            break;
        case WM_RBUTTONDOWN:
            newMouseButtonsState |= MK_RBUTTON;
            break;
        case WM_RBUTTONUP:
            newMouseButtonsState &= ~MK_RBUTTON;
            break;
        case WM_MBUTTONDOWN:
            newMouseButtonsState |= MK_MBUTTON;
            break;
        case WM_MBUTTONUP:
            newMouseButtonsState &= ~MK_MBUTTON;
            break;
        case WM_XBUTTONDOWN:
            newMouseButtonsState |= (xbutton == XBUTTON1 ? MK_XBUTTON1 : MK_XBUTTON2);
            break;
        case WM_XBUTTONUP:
            newMouseButtonsState &= ~(xbutton == XBUTTON1 ? MK_XBUTTON1 : MK_XBUTTON2);
            break;
        default:
            return 0;
        }

        bool isPressed = false;
        eMouseButtons button = GetMouseButtonLegacy(mouseButtonsState, newMouseButtonsState, &isPressed);
        if (button != eMouseButtons::NONE)
        {
            eModifierKeys modifierKeys = GetModifierKeys();
            float32 vx = static_cast<float32>(x);
            float32 vy = static_cast<float32>(y);
            MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, vx, vy, 1, modifierKeys, false));

            bool setCapture = newMouseButtonsState != 0 && mouseButtonsState == 0;
            mouseButtonsState = newMouseButtonsState;
            if (setCapture)
            {
                ::SetCapture(hwnd);
            }
            else if (newMouseButtonsState == 0)
            {
                ::ReleaseCapture();
            }
        }
    }
    return 0;
}

LRESULT WindowBackend::OnCaptureChanged()
{
    if (mouseButtonsState != 0)
    {
        POINT pt;
        ::GetCursorPos(&pt);
        ::ScreenToClient(hwnd, &pt);

        eModifierKeys modifierKeys = GetModifierKeys();
        float32 vx = static_cast<float32>(pt.x);
        float32 vy = static_cast<float32>(pt.y);
        MainDispatcherEvent e = MainDispatcherEvent::CreateWindowMouseClickEvent(window, MainDispatcherEvent::MOUSE_BUTTON_UP, eMouseButtons::LEFT, vx, vy, 1, modifierKeys, false);

        if (mouseButtonsState & MK_LBUTTON)
        {
            e.mouseEvent.button = eMouseButtons::LEFT;
            mainDispatcher->PostEvent(e);
        }
        if (mouseButtonsState & MK_RBUTTON)
        {
            e.mouseEvent.button = eMouseButtons::RIGHT;
            mainDispatcher->PostEvent(e);
        }
        if (mouseButtonsState & MK_MBUTTON)
        {
            e.mouseEvent.button = eMouseButtons::MIDDLE;
            mainDispatcher->PostEvent(e);
        }
        if (mouseButtonsState & MK_XBUTTON1)
        {
            e.mouseEvent.button = eMouseButtons::EXTENDED1;
            mainDispatcher->PostEvent(e);
        }
        if (mouseButtonsState & MK_XBUTTON2)
        {
            e.mouseEvent.button = eMouseButtons::EXTENDED2;
            mainDispatcher->PostEvent(e);
        }

        mouseButtonsState = 0;
    }
    return 0;
}

LRESULT WindowBackend::OnTouch(uint32 ntouch, HTOUCHINPUT htouch)
{
    touchInput.resize(ntouch);
    TOUCHINPUT* pinput = touchInput.data();
    if (::GetTouchInputInfo(htouch, ntouch, pinput, sizeof(TOUCHINPUT)))
    {
        eModifierKeys modifierKeys = GetModifierKeys();
        MainDispatcherEvent e = MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_MOVE, 0, 0.f, 0.f, modifierKeys);
        for (TOUCHINPUT& touch : touchInput)
        {
            POINT pt = { touch.x / 100, touch.y / 100 };
            ::ScreenToClient(hwnd, &pt);
            if (touch.dwFlags & (TOUCHEVENTF_PRIMARY | TOUCHEVENTF_MOVE))
            {
                // Remember move position of primary touch point to skip spurious move events as
                // Windows generates WM_MOUSEMOVE event for primary touch point.
                lastMouseMoveX = pt.x;
                lastMouseMoveY = pt.y;
            }

            if (touch.dwFlags & TOUCHEVENTF_MOVE)
                e.type = MainDispatcherEvent::TOUCH_MOVE;
            else if (touch.dwFlags & TOUCHEVENTF_DOWN)
                e.type = MainDispatcherEvent::TOUCH_DOWN;
            else if (touch.dwFlags & TOUCHEVENTF_UP)
                e.type = MainDispatcherEvent::TOUCH_UP;
            else
                continue;
            e.touchEvent.touchId = static_cast<uint32>(touch.dwID);
            e.touchEvent.x = static_cast<float32>(pt.x);
            e.touchEvent.y = static_cast<float32>(pt.y);
            mainDispatcher->PostEvent(e);
        }
        ::CloseTouchInputHandle(htouch);
    }
    return 0;
}

LRESULT WindowBackend::OnPointerClick(uint32 pointerId, int32 x, int32 y)
{
    POINTER_INFO pointerInfo;
    DllImport::fnGetPointerInfo(pointerId, &pointerInfo);

    bool isPressed = false;
    float32 vx = static_cast<float32>(x);
    float32 vy = static_cast<float32>(y);
    eModifierKeys modifierKeys = GetModifierKeys();
    if (pointerInfo.pointerType == PT_MOUSE)
    {
        eMouseButtons button = GetMouseButton(pointerInfo.ButtonChangeType, &isPressed);
        MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, vx, vy, 1, modifierKeys, false));
    }
    else if (pointerInfo.pointerType == PT_TOUCH)
    {
        isPressed = (pointerInfo.pointerFlags & POINTER_FLAG_DOWN) == POINTER_FLAG_DOWN;
        MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::TOUCH_DOWN : MainDispatcherEvent::TOUCH_UP;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, type, pointerId, vx, vy, modifierKeys));
    }
    return 0;
}

LRESULT WindowBackend::OnPointerUpdate(uint32 pointerId, int32 x, int32 y)
{
    POINTER_INFO pointerInfo;
    DllImport::fnGetPointerInfo(pointerId, &pointerInfo);

    float32 vx = static_cast<float32>(x);
    float32 vy = static_cast<float32>(y);
    eModifierKeys modifierKeys = GetModifierKeys();
    if (pointerInfo.pointerType == PT_MOUSE)
    {
        if (pointerInfo.ButtonChangeType != POINTER_CHANGE_NONE)
        {
            // First mouse button down (and last mouse button up) comes with WM_POINTERDOWN/WM_POINTERUP, other mouse clicks come here
            bool isPressed = false;
            eMouseButtons button = GetMouseButton(pointerInfo.ButtonChangeType, &isPressed);
            MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, vx, vy, 1, modifierKeys, false));
        }
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, vx, vy, modifierKeys, false));
    }
    else if (pointerInfo.pointerType == PT_TOUCH)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_MOVE, pointerId, vx, vy, modifierKeys));
    }
    return 0;
}

LRESULT WindowBackend::OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated)
{
    // How to distinguish left and right shift, control and alt
    // http://stackoverflow.com/a/15977613
    if (isExtended || (key == VK_SHIFT && ::MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT))
    {
        key |= 0x100;
    }

    eModifierKeys modifierKeys = GetModifierKeys();
    MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, modifierKeys, isRepeated));
    return 0;
}

LRESULT WindowBackend::OnCharEvent(uint32 key, bool isRepeated)
{
    eModifierKeys modifierKeys = GetModifierKeys();
    // Windows translates some Ctrl key combinations into ASCII control characters.
    // It seems to me that control character are not wanted by game to handle in character message.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/gg153546(v=vs.85).aspx
    if ((modifierKeys & eModifierKeys::CONTROL) == eModifierKeys::NONE)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, key, modifierKeys, isRepeated));
    }
    return 0;
}

LRESULT WindowBackend::OnCreate()
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);

    // If new pointer input is available then do not handle legacy WM_TOUCH message
    if (DllImport::fnIsMouseInPointerEnabled == nullptr || !DllImport::fnIsMouseInPointerEnabled())
    {
        ::RegisterTouchWindow(hwnd, TWF_FINETOUCH | TWF_WANTPALM);
    }

    width = rc.right - rc.left;
    height = rc.bottom - rc.top;
    float32 w = static_cast<float32>(width);
    float32 h = static_cast<float32>(height);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, w, h, 1.0f, 1.0f));
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
        int32 w = GET_X_LPARAM(lparam);
        int32 h = GET_Y_LPARAM(lparam);
        lresult = OnSize(static_cast<int32>(wparam), w, h);
    }
    else if (message == WM_ERASEBKGND)
    {
        lresult = 1;
    }
    else if (message == WM_GETMINMAXINFO)
    {
        MINMAXINFO* minMaxInfo = reinterpret_cast<MINMAXINFO*>(lparam);
        lresult = OnGetMinMaxInfo(minMaxInfo);
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
    else if (message == WM_POINTERDOWN || message == WM_POINTERUP)
    {
        uint32 pointerId = GET_POINTERID_WPARAM(wparam);
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        ::ScreenToClient(hwnd, &pt);

        lresult = OnPointerClick(pointerId, pt.x, pt.y);
    }
    else if (message == WM_POINTERUPDATE)
    {
        uint32 pointerId = GET_POINTERID_WPARAM(wparam);
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        ::ScreenToClient(hwnd, &pt);

        lresult = OnPointerUpdate(pointerId, pt.x, pt.y);
    }
    else if (message == WM_POINTERWHEEL || message == WM_POINTERHWHEEL)
    {
        int32 deltaX = 0;
        int32 deltaY = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
        if (message == WM_POINTERHWHEEL)
        {
            using std::swap;
            std::swap(deltaX, deltaY);
        }
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        ::ScreenToClient(hwnd, &pt);

        lresult = OnMouseWheelEvent(deltaX, deltaY, pt.x, pt.y);
    }
    else if (message == WM_MOUSEMOVE)
    {
        int32 x = GET_X_LPARAM(lparam);
        int32 y = GET_Y_LPARAM(lparam);
        lresult = OnMouseMoveEvent(x, y);
    }
    else if (message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL)
    {
        int32 deltaX = 0;
        int32 deltaY = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
        if (message == WM_MOUSEHWHEEL)
        {
            using std::swap;
            std::swap(deltaX, deltaY);
        }
        int32 x = GET_X_LPARAM(lparam);
        int32 y = GET_Y_LPARAM(lparam);
        lresult = OnMouseWheelEvent(deltaX, deltaY, x, y);
    }
    else if (WM_MOUSEFIRST <= message && message <= WM_MOUSELAST)
    {
        uint16 xbutton = GET_XBUTTON_WPARAM(wparam);
        int32 x = GET_X_LPARAM(lparam);
        int32 y = GET_Y_LPARAM(lparam);
        lresult = OnMouseClickEvent(message, xbutton, x, y);
    }
    else if (message == WM_CAPTURECHANGED)
    {
        lresult = OnCaptureChanged();
    }
    else if (message == WM_TOUCH)
    {
        uint32 ntouch = LOWORD(wparam);
        HTOUCHINPUT htouch = reinterpret_cast<HTOUCHINPUT>(lparam);
        lresult = OnTouch(ntouch, htouch);
    }
    else if (message == WM_KEYUP || message == WM_KEYDOWN || message == WM_SYSKEYUP || message == WM_SYSKEYDOWN)
    {
        uint32 key = static_cast<uint32>(wparam);
        uint32 scanCode = (static_cast<uint32>(lparam) >> 16) & 0xFF;
        bool isPressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
        bool isExtended = (HIWORD(lparam) & KF_EXTENDED) == KF_EXTENDED;
        bool isRepeated = (HIWORD(lparam) & KF_REPEAT) == KF_REPEAT;
        lresult = OnKeyEvent(key, scanCode, isPressed, isExtended, isRepeated);
        // Forward WM_SYSKEYUP and WM_SYSKEYDOWN to DefWindowProc to allow system shortcuts: Alt+F4, etc
        isHandled = (message == WM_KEYUP || message == WM_KEYDOWN);
    }
    else if (message == WM_UNICHAR)
    {
        uint32 key = static_cast<uint32>(wparam);
        if (key != UNICODE_NOCHAR)
        {
            bool isRepeated = (HIWORD(lparam) & KF_REPEAT) == KF_REPEAT;
            lresult = OnCharEvent(key, isRepeated);
        }
        else
        {
            lresult = TRUE;
        }
    }
    else if (message == WM_CHAR || message == WM_SYSCHAR)
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

eModifierKeys WindowBackend::GetModifierKeys()
{
    eModifierKeys result = eModifierKeys::NONE;
    BYTE keyState[256];
    if (::GetKeyboardState(keyState))
    {
        if ((keyState[VK_LSHIFT] & 0x80) || (keyState[VK_RSHIFT] & 0x80))
        {
            result |= eModifierKeys::SHIFT;
        }
        if ((keyState[VK_LCONTROL] & 0x80) || (keyState[VK_RCONTROL] & 0x80))
        {
            result |= eModifierKeys::CONTROL;
        }
        if ((keyState[VK_LMENU] & 0x80) || (keyState[VK_RMENU] & 0x80))
        {
            result |= eModifierKeys::ALT;
        }
    }
    return result;
}

eInputDevice WindowBackend::GetInputEventSourceLegacy(LPARAM messageExtraInfo)
{
    // How to distinguish pen input from mouse and touch
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms703320(v=vs.85).aspx

    const LPARAM MI_WP_SIGNATURE = 0xFF515700;
    const LPARAM SIGNATURE_MASK = 0xFFFFFF00;

    if ((messageExtraInfo & SIGNATURE_MASK) == MI_WP_SIGNATURE)
    {
        return eInputDevice::TOUCH_SURFACE;
    }
    return eInputDevice::MOUSE;
}

eMouseButtons WindowBackend::GetMouseButtonLegacy(uint32 curState, uint32 newState, bool* isPressed)
{
    uint32 changed = curState ^ newState;
    if (changed & MK_LBUTTON)
    {
        *isPressed = (newState & MK_LBUTTON) != 0;
        return eMouseButtons::LEFT;
    }
    if (changed & MK_RBUTTON)
    {
        *isPressed = (newState & MK_RBUTTON) != 0;
        return eMouseButtons::RIGHT;
    }
    if (changed & MK_MBUTTON)
    {
        *isPressed = (newState & MK_MBUTTON) != 0;
        return eMouseButtons::MIDDLE;
    }
    if (changed & MK_XBUTTON1)
    {
        *isPressed = (newState & MK_XBUTTON1) != 0;
        return eMouseButtons::EXTENDED1;
    }
    if (changed & MK_XBUTTON2)
    {
        *isPressed = (newState & MK_XBUTTON2) != 0;
        return eMouseButtons::EXTENDED2;
    }
    return eMouseButtons::NONE;
}

eMouseButtons WindowBackend::GetMouseButton(POINTER_BUTTON_CHANGE_TYPE buttonChangeType, bool* isPressed)
{
    *isPressed = false;
    switch (buttonChangeType)
    {
    case POINTER_CHANGE_FIRSTBUTTON_DOWN:
        *isPressed = true;
    case POINTER_CHANGE_FIRSTBUTTON_UP:
        return eMouseButtons::LEFT;
    case POINTER_CHANGE_SECONDBUTTON_DOWN:
        *isPressed = true;
    case POINTER_CHANGE_SECONDBUTTON_UP:
        return eMouseButtons::RIGHT;
    case POINTER_CHANGE_THIRDBUTTON_DOWN:
        *isPressed = true;
    case POINTER_CHANGE_THIRDBUTTON_UP:
        return eMouseButtons::MIDDLE;
    case POINTER_CHANGE_FOURTHBUTTON_DOWN:
        *isPressed = true;
    case POINTER_CHANGE_FOURTHBUTTON_UP:
        return eMouseButtons::EXTENDED1;
    case POINTER_CHANGE_FIFTHBUTTON_DOWN:
        *isPressed = true;
    case POINTER_CHANGE_FIFTHBUTTON_UP:
        return eMouseButtons::EXTENDED2;
    default:
        return eMouseButtons::NONE;
    }
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
