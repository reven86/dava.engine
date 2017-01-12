#if !defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_WIN32__)

#include "Base/Platform.h"
#include <shellapi.h>
#include <thread>

#include "WinSystemTimer.h"
#include "Concurrency/Thread.h"
#include "Input/KeyboardDevice.h"
#include "Input/InputSystem.h"
#include "Platform/DPIHelper.h"
#include "Platform/DeviceInfo.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"
#include "Time/SystemTimer.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UIControlSystem.h"
#include "Utils/UTF8Utils.h"
#if defined(__DAVAENGINE_STEAM__)
#include "Platform/Steam.h"
#endif

#include "MemoryManager/MemoryProfiler.h"
#include "Logger/Logger.h"
#include "Debug/DVAssertDefaultHandlers.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{
const UINT MSG_ALREADY_RUNNING = ::RegisterWindowMessage(L"MSG_ALREADY_RUNNING");
bool AlreadyRunning();
void ShowRunningApplication();
uint32 GetKeyboardModifiers();

int Core::Run(int argc, char* argv[], AppHandle handle)
{
    Assert::SetupDefaultHandlers();

#if defined(DENY_RUN_MULTIPLE_APP_INSTANCES)
    if (AlreadyRunning())
    {
        ShowRunningApplication();
        return 0;
    }
#endif
    SetProcessDPIAware();
    CoreWin32Platform* core = new CoreWin32Platform();
    core->InitArgs();
    core->CreateSingletons();

    bool windowCreated = core->CreateWin32Window(handle);
    if (windowCreated)
    {
        core->Run();
        core->ReleaseSingletons();
    }

    DAVA_MEMORY_PROFILER_FINISH();
    return 0;
}

int Core::RunCmdTool(int argc, char* argv[], AppHandle handle)
{
    CoreWin32Platform* core = new CoreWin32Platform();
    core->InitArgs();

    core->EnableConsoleMode();
    core->CreateSingletons();

    Logger::Instance()->EnableConsoleMode();

    FrameworkDidLaunched();
    FrameworkWillTerminate();
    core->ReleaseSingletons();

    DAVA_MEMORY_PROFILER_FINISH();
    return 0;
}

CoreWin32Platform::CoreWin32Platform()
{
    SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
}

bool CoreWin32Platform::CreateWin32Window(HINSTANCE hInstance)
{
    //single instance check
    TCHAR fileName[MAX_PATH];
    GetModuleFileName(NULL, fileName, MAX_PATH);
    fileName[MAX_PATH - 1] = 0; //string can be not null-terminated on winXP
    for (int32 i = 0; i < MAX_PATH; ++i)
    {
        if (fileName[i] == L'\\') //symbol \ is not allowed in CreateMutex mutex name
        {
            fileName[i] = ' ';
        }
    }
    SetLastError(0);

    windowedMode = DisplayMode(800, 600, 16, 0);
    fullscreenMode = DisplayMode(800, 600, 16, 0);
    currentMode = windowedMode;
    isFullscreen = false;

    // create the window, only if we do not use the null device
    LPCWSTR className = L"DavaFrameworkWindowsDevice";

    // Register Class

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = 0;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = className;
    wcex.hIconSm = 0;

    RegisterClassEx(&wcex);

    // calculate client size

    RECT clientSize;
    clientSize.top = 0;
    clientSize.left = 0;
    clientSize.right = currentMode.width;
    clientSize.bottom = currentMode.height;

    ULONG style = WINDOWED_STYLE | WS_CLIPCHILDREN;

    // Create the rendering window
    if (isFullscreen)
    {
        style = FULLSCREEN_STYLE;
    } // End if Fullscreen

    AdjustWindowRect(&clientSize, style, FALSE);

    int32 realWidth = clientSize.right - clientSize.left;
    int32 realHeight = clientSize.bottom - clientSize.top;

    int32 windowLeft = -10000; //(GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
    int32 windowTop = -10000; //(GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

    if (isFullscreen)
    {
        windowLeft = 0;
        windowTop = 0;
    }

    // create window
    hWindow = CreateWindow(className, L"", style, windowLeft, windowTop,
                           realWidth, realHeight, NULL, NULL, hInstance, NULL);

    SetMenu(hWindow, NULL);

    // fix ugly ATI driver bugs. Thanks to ariaci (Taken from Irrlight).
    MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);

    FrameworkDidLaunched();
#if defined(__DAVAENGINE_STEAM__)
    Steam::Init();
#endif

    KeyedArchive* options = Core::GetOptions();

    bool shouldEnableFullscreen = false;

    DWORD iModeNum = 0;
    DEVMODE dmi;
    ZeroMemory(&dmi, sizeof(dmi));
    dmi.dmSize = sizeof(dmi);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmi))
    {
        fullscreenMode.width = dmi.dmPelsWidth;
        fullscreenMode.height = dmi.dmPelsHeight;
        fullscreenMode.bpp = dmi.dmBitsPerPel;
        fullscreenMode.refreshRate = dmi.dmDisplayFrequency;
        ZeroMemory(&dmi, sizeof(dmi));
    }
    // calculate window area
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int32 workWidth = workArea.right - workArea.left;
    int32 workHeight = workArea.bottom - workArea.top;
    // calculate border
    RECT borderRect = { 0, 0, 0, 0 };
    AdjustWindowRect(&borderRect, style, 0);
    int32 borderWidth = borderRect.right - borderRect.left;
    int32 borderHeight = borderRect.bottom - borderRect.top;
    int32 maxWidth = workWidth - borderWidth;
    int32 maxHeight = workHeight - borderHeight;

    if (options)
    {
        windowedMode.width = options->GetInt32("width");
        windowedMode.height = options->GetInt32("height");
        windowedMode.bpp = options->GetInt32("bpp");

        //check windowed sizes
        if (windowedMode.width > maxWidth)
        {
            windowedMode.width = maxWidth;
        }
        if (windowedMode.height > maxHeight)
        {
            windowedMode.height = maxHeight;
        }

        // get values from config in case if they are available
        fullscreenMode.width = options->GetInt32("fullscreen.width", fullscreenMode.width);
        fullscreenMode.height = options->GetInt32("fullscreen.height", fullscreenMode.height);
        fullscreenMode.bpp = windowedMode.bpp;

        fullscreenMode = FindBestMode(fullscreenMode);
        shouldEnableFullscreen = options->GetInt32("fullscreen", 0) == 1;
        String title = options->GetString("title", "[set application title using core options property 'title']");
        WideString titleW = UTF8Utils::EncodeToWideString(title);
        SetWindowText(hWindow, titleW.c_str());

        LoadWindowMinimumSizeSettings();
    }

    Logger::FrameworkDebug("[PlatformWin32] best display fullscreen mode matched: %d x %d x %d refreshRate: %d", fullscreenMode.width, fullscreenMode.height, fullscreenMode.bpp, fullscreenMode.refreshRate);

    // Init application with positioned window
    {
        currentMode = windowedMode;
        Core::Instance()->InitWindowSize(reinterpret_cast<void*>(hWindow), static_cast<float32>(currentMode.width), static_cast<float32>(currentMode.height), 1.f, 1.f);

        clientSize.top = 0;
        clientSize.left = 0;
        clientSize.right = currentMode.width;
        clientSize.bottom = currentMode.height;

        AdjustWindowRect(&clientSize, style, FALSE);

        realWidth = clientSize.right - clientSize.left;
        realHeight = clientSize.bottom - clientSize.top;

        windowLeft = (workWidth - realWidth) / 2 + workArea.left;
        windowTop = (workHeight - realHeight) / 2 + workArea.top;

        MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);
    }

    if (shouldEnableFullscreen)
    {
        SetScreenMode(eScreenMode::FULLSCREEN);
    }

    RAWINPUTDEVICE Rid;

    Rid.usUsagePage = 0x01;
    Rid.usUsage = 0x02;
    Rid.dwFlags = 0;
    Rid.hwndTarget = 0;

    RegisterRawInputDevices(&Rid, 1, sizeof(Rid));

    int value = GetSystemMetrics(SM_DIGITIZER);
    if (value & NID_READY)
    {
        if (!RegisterTouchWindow(hWindow, 0))
        {
            Logger::Error("can't register touch window");
        }
    }
    else
    {
        Logger::Info("system not supported touch input");
    }

    UIControlSystem::Instance()->vcs->SetInputScreenAreaSize(currentMode.width, currentMode.height);
    UIControlSystem::Instance()->vcs->SetPhysicalScreenSize(currentMode.width, currentMode.height);

    return true;
}

void CoreWin32Platform::LoadWindowMinimumSizeSettings()
{
    const KeyedArchive* options = GetOptions();
    int32 minWidth = options->GetInt32("min-width", 0);
    int32 minHeight = options->GetInt32("min-height", 0);
    if (minWidth > 0 && minHeight > 0)
    {
        SetWindowMinimumSize(static_cast<float32>(minWidth), static_cast<float32>(minHeight));
    }
}

void CoreWin32Platform::ClearMouseButtons()
{
    UIEvent e;

    e.phase = UIEvent::Phase::ENDED;
    e.device = eInputDevices::MOUSE;
    e.timestamp = (SystemTimer::GetMs() / 1000.f);
    e.modifiers = GetKeyboardModifiers();

    for (uint32 mouseButton = static_cast<uint32>(eMouseButtons::FIRST);
         mouseButton <= static_cast<uint32>(eMouseButtons::LAST);
         mouseButton += 1)
    {
        if (mouseButtonState[mouseButton - 1])
        {
            e.mouseButton = static_cast<eMouseButtons>(mouseButton);

            UIControlSystem::Instance()->OnInput(&e);
        }
    }

    mouseButtonState.reset();
}

void CoreWin32Platform::Run()
{
    Instance()->SystemAppStarted();
    // after call ShowWindow, the system sends message WM_ACTIVATE.
    // do it after ApplicationCore->OnAppStarted, because otherwise we call client code before it initialized
    HWND hWindow = static_cast<HWND>(Instance()->GetNativeView());
    ShowWindow(hWindow, SW_SHOW);
    UpdateWindow(hWindow);

    MSG msg;
    while (1)
    {
        uint64 startTime = SystemTimer::GetMs();

        // process messages
        willQuit = false;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                ApplicationCore* appCore = Instance()->GetApplicationCore();
                if (appCore && appCore->OnQuit())
                {
                    exit(0);
                }
                else
                {
                    willQuit = true;
                }
            }
        }

        SystemProcessFrame();

        int32 fps = Renderer::GetDesiredFPS();
        if (fps > 0)
        {
            int32 elapsedTime = static_cast<int32>(SystemTimer::GetMs() - startTime);
            int32 sleepMs = (1000 / fps) - elapsedTime;
            if (sleepMs > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
            }
        }

        if (willQuit)
        {
            break;
        }
    }

    Core::Instance()->FocusLost();
    Core::Instance()->GoBackground(false);

    Core::Instance()->SystemAppFinished();
#if defined(__DAVAENGINE_STEAM__)
    Steam::Deinit();
#endif
    FrameworkWillTerminate();
}

RECT CoreWin32Platform::GetWindowedRectForDisplayMode(DisplayMode& dm)
{
    RECT clientSize;
    clientSize.top = 0;
    clientSize.left = 0;
    clientSize.right = dm.width;
    clientSize.bottom = dm.height;
    HWND hWindow = static_cast<HWND>(GetNativeView());
    AdjustWindowRect(&clientSize, GetWindowLong(hWindow, GWL_STYLE), FALSE);

    return clientSize;
}

Core::eScreenMode CoreWin32Platform::GetScreenMode()
{
    if (isFullscreen)
    {
        return Core::eScreenMode::FULLSCREEN;
    }
    else
    {
        return Core::eScreenMode::WINDOWED;
    }
}

bool CoreWin32Platform::SetScreenMode(eScreenMode screenMode)
{
    // How do I switch a window between normal and fullscreen?
    // https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353/

    static WINDOWPLACEMENT windowPlacement = { sizeof(WINDOWPLACEMENT) };
    if (GetScreenMode() != screenMode)
    {
        HWND hWindow = static_cast<HWND>(GetNativeView());
        switch (screenMode)
        {
        case DAVA::Core::eScreenMode::FULLSCREEN:
        {
            isFullscreen = true;
            currentMode = fullscreenMode;

            GetWindowPlacement(hWindow, &windowPlacement);

            // Add WS_VISIBLE to fullscreen style to keep it visible (if it already is)
            // If it's not yet visible, the style should not be modified since ShowWindow(..., SW_SHOW) will occur later
            //
            uint32 style = FULLSCREEN_STYLE;
            if (IsWindowVisible(hWindow))
            {
                style |= WS_VISIBLE;
            }
            SetWindowLong(hWindow, GWL_STYLE, style);

            MONITORINFO monitorInfo;
            monitorInfo.cbSize = sizeof(monitorInfo);
            GetMonitorInfo(MonitorFromWindow(hWindow, MONITOR_DEFAULTTONEAREST), &monitorInfo);
            RECT rect(monitorInfo.rcMonitor);

            SetWindowPos(hWindow,
                         HWND_TOP,
                         rect.left,
                         rect.top,
                         rect.right - rect.left,
                         rect.bottom - rect.top,
                         SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
            break;
        }
        case DAVA::Core::eScreenMode::WINDOWED_FULLSCREEN:
        {
            Logger::Error("Unsupported screen mode");
            return false;
        }
        case DAVA::Core::eScreenMode::WINDOWED:
        {
            isFullscreen = false;
            currentMode = windowedMode;

            SetWindowLong(hWindow, GWL_STYLE, WINDOWED_STYLE);
            SetWindowPlacement(hWindow, &windowPlacement);
            SetWindowPos(hWindow,
                         nullptr,
                         0, 0, 0, 0,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            break;
        }
        default:
        {
            DVASSERT(false, "Incorrect screen mode");
            Logger::Error("Incorrect screen mode");
            return false;
        }
        }

        Logger::FrameworkDebug("[CoreWin32Platform] toggle mode: %d x %d isFullscreen: %d", currentMode.width, currentMode.height, isFullscreen);
        UIControlSystem::Instance()->vcs->SetInputScreenAreaSize(currentMode.width, currentMode.height);
        UIControlSystem::Instance()->vcs->SetPhysicalScreenSize(currentMode.width, currentMode.height);
    }
    return true;
}

void CoreWin32Platform::GetAvailableDisplayModes(List<DisplayMode>& availableDisplayModes)
{
    availableDisplayModes.clear();

    DWORD iModeNum = 0;
    DEVMODE dmi;
    ZeroMemory(&dmi, sizeof(dmi));
    dmi.dmSize = sizeof(dmi);

    while (EnumDisplaySettings(NULL, iModeNum++, &dmi))
    {
        DisplayMode mode;
        mode.width = dmi.dmPelsWidth;
        mode.height = dmi.dmPelsHeight;
        mode.bpp = dmi.dmBitsPerPel;
        mode.refreshRate = dmi.dmDisplayFrequency;
        ZeroMemory(&dmi, sizeof(dmi));
        availableDisplayModes.push_back(mode);
    }
}

void CoreWin32Platform::SetIcon(int32 iconId)
{
    HWND hWindow = static_cast<HWND>(GetNativeView());
    HINSTANCE hInst = GetModuleHandle(0);
    HICON smallIcon = static_cast<HICON>(LoadImage(hInst,
                                                   MAKEINTRESOURCE(iconId),
                                                   IMAGE_ICON,
                                                   0,
                                                   0,
                                                   LR_DEFAULTSIZE));
    SendMessage(hWindow, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
    SendMessage(hWindow, WM_SETICON, ICON_BIG, (LPARAM)smallIcon);
}

void CoreWin32Platform::SetWindowMinimumSize(float32 width, float32 height)
{
    DVASSERT((width == 0.0f && height == 0.0f) || (width > 0.0f && height > 0.0f));
    minWindowWidth = width;
    minWindowHeight = height;
}

Vector2 CoreWin32Platform::GetWindowMinimumSize() const
{
    return Vector2(minWindowWidth, minWindowHeight);
}

void CoreWin32Platform::OnMouseMove(int32 x, int32 y)
{
    UIEvent e;
    e.physPoint = Vector2(static_cast<float32>(x), static_cast<float32>(y));
    e.device = eInputDevices::MOUSE;
    e.timestamp = (SystemTimer::GetMs() / 1000.0);
    e.modifiers = GetKeyboardModifiers();

    if (mouseButtonState.any())
    {
        for (unsigned buttonIndex = static_cast<unsigned>(eMouseButtons::FIRST);
             buttonIndex <= static_cast<unsigned>(eMouseButtons::LAST);
             ++buttonIndex)
        {
            if (mouseButtonState[buttonIndex - 1])
            {
                e.mouseButton = static_cast<eMouseButtons>(buttonIndex);
                e.phase = UIEvent::Phase::DRAG;
                UIControlSystem::Instance()->OnInput(&e);
            }
        }
    }
    else
    {
        e.mouseButton = eMouseButtons::NONE;
        e.phase = UIEvent::Phase::MOVE;
        UIControlSystem::Instance()->OnInput(&e);
    }
}

void CoreWin32Platform::OnMouseWheel(int32 wheelDeltaX, int32 wheelDeltaY, int32 x, int32 y)
{
    UIEvent e;
    e.physPoint = Vector2(static_cast<float32>(x), static_cast<float32>(y));
    e.device = eInputDevices::MOUSE;
    e.phase = UIEvent::Phase::WHEEL;
    e.timestamp = (SystemTimer::GetMs() / 1000.0);
    e.modifiers = GetKeyboardModifiers();
    e.wheelDelta = { static_cast<float32>(wheelDeltaX), static_cast<float32>(wheelDeltaY) };

    UIControlSystem::Instance()->OnInput(&e);
}

void CoreWin32Platform::OnMouseClick(UIEvent::Phase phase, eMouseButtons button, int32 x, int32 y)
{
    bool isButtonDown = phase == UIEvent::Phase::BEGAN;
    unsigned buttonIndex = static_cast<unsigned>(button) - 1;

    bool isAnyButtonDownBefore = mouseButtonState.any();

    mouseButtonState[buttonIndex] = isButtonDown;

    UIEvent e;
    e.physPoint = Vector2(static_cast<float32>(x), static_cast<float32>(y));
    e.device = eInputDevices::MOUSE;
    e.phase = phase;
    e.mouseButton = button;
    e.timestamp = (SystemTimer::GetMs() / 1000.0);
    e.modifiers = GetKeyboardModifiers();

    UIControlSystem::Instance()->OnInput(&e);

    bool isAnyButtonDownAfter = mouseButtonState.any();

    if (isAnyButtonDownBefore && !isAnyButtonDownAfter)
    {
        ReleaseCapture();
    }
    else if (!isAnyButtonDownBefore && isAnyButtonDownAfter)
    {
        SetCapture(hWindow);
    }
}

void CoreWin32Platform::OnTouchEvent(UIEvent::Phase phase, eInputDevices deviceId, uint32 fingerId, float32 x, float32 y, float presure)
{
    UIEvent newTouch;
    newTouch.touchId = fingerId;
    newTouch.physPoint = Vector2(x, y);
    newTouch.phase = phase;
    newTouch.device = deviceId;
    newTouch.timestamp = (SystemTimer::GetMs() / 1000.0);
    newTouch.modifiers = GetKeyboardModifiers();

    UIControlSystem::Instance()->OnInput(&newTouch);
}

void CoreWin32Platform::OnGetMinMaxInfo(MINMAXINFO* minmaxInfo)
{
    if (minWindowWidth > 0.0f && minWindowHeight > 0.0f)
    {
        DWORD style = static_cast<DWORD>(GetWindowLongPtr(hWindow, GWL_STYLE));
        DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(hWindow, GWL_EXSTYLE));

        RECT rc = {
            0, 0,
            static_cast<LONG>(minWindowWidth),
            static_cast<LONG>(minWindowHeight)
        };
        // dava.engine's clients expect minimum window size for client area not window frame
        AdjustWindowRectEx(&rc, style, FALSE, exStyle);

        minmaxInfo->ptMinTrackSize.x = rc.right - rc.left;
        minmaxInfo->ptMinTrackSize.y = rc.bottom - rc.top;
    }
}

bool IsMouseClickEvent(UINT message)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        return true;
    default:
        return false;
    }
}

bool IsMouseMoveEvent(UINT message)
{
    return message == WM_INPUT;
}

bool IsMouseWheelEvent(UINT message)
{
    return message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL;
}

bool IsMouseInputEvent(UINT message, LPARAM messageExtraInfo)
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms703320(v=vs.85).aspx

    const LPARAM MI_WP_SIGNATURE = 0xFF515700;
    const LPARAM SIGNATURE_MASK = 0xFFFFFF00;
    const bool isTouchOrPen = (messageExtraInfo & SIGNATURE_MASK) == MI_WP_SIGNATURE;

    return !isTouchOrPen && ((WM_MOUSEFIRST <= message && message <= WM_MOUSELAST) || message == WM_INPUT);
}

bool IsCursorPointInside(HWND hWnd, int xPos, int yPos)
{
    if (InputSystem::Instance()->GetMouseDevice().IsPinningEnabled())
    {
        return true;
    }
    else
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        bool isInside = xPos > clientRect.left && xPos < clientRect.right &&
        yPos > clientRect.top && yPos < clientRect.bottom;

        return isInside;
    }
}

bool CoreWin32Platform::ProcessMouseClickEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lPampam)
{
    int xPos = GET_X_LPARAM(lPampam);
    int yPos = GET_Y_LPARAM(lPampam);

    eMouseButtons button = eMouseButtons::NONE;
    eMouseButtons extButton = eMouseButtons::NONE;
    UIEvent::Phase phase = UIEvent::Phase::ERROR;

    if (message == WM_LBUTTONDOWN)
    {
        button = eMouseButtons::LEFT;
        phase = UIEvent::Phase::BEGAN;
    }
    else if (message == WM_LBUTTONUP)
    {
        button = eMouseButtons::LEFT;
        phase = UIEvent::Phase::ENDED;
    }
    else if (message == WM_LBUTTONDBLCLK)
    {
        button = eMouseButtons::LEFT;
        phase = UIEvent::Phase::ENDED;
    }
    else if (message == WM_RBUTTONDOWN)
    {
        button = eMouseButtons::RIGHT;
        phase = UIEvent::Phase::BEGAN;
    }
    else if (message == WM_RBUTTONUP)
    {
        button = eMouseButtons::RIGHT;
        phase = UIEvent::Phase::ENDED;
    }
    else if (message == WM_RBUTTONDBLCLK)
    {
        button = eMouseButtons::RIGHT;
        phase = UIEvent::Phase::ENDED;
    }
    else if (message == WM_MBUTTONDOWN)
    {
        button = eMouseButtons::MIDDLE;
        phase = UIEvent::Phase::BEGAN;
    }
    else if (message == WM_MBUTTONUP)
    {
        button = eMouseButtons::MIDDLE;
        phase = UIEvent::Phase::ENDED;
    }
    else if (message == WM_MBUTTONDBLCLK)
    {
        button = eMouseButtons::MIDDLE;
        phase = UIEvent::Phase::ENDED;
    }

    else if (message == WM_XBUTTONDOWN || message == WM_XBUTTONUP || message == WM_XBUTTONDBLCLK)
    {
        phase = message == WM_XBUTTONDOWN ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
        int xButton = GET_XBUTTON_WPARAM(wParam);

        if ((xButton & XBUTTON1) != 0)
        {
            button = eMouseButtons::EXTENDED1;
        }

        if ((xButton & XBUTTON2) != 0)
        {
            extButton = eMouseButtons::EXTENDED2;
        }
    }

    if (button != eMouseButtons::NONE)
    {
        OnMouseClick(phase, button, xPos, yPos);
        return true;
    }
    if (extButton != eMouseButtons::NONE)
    {
        OnMouseClick(phase, extButton, xPos, yPos);
        return true;
    }

    return false;
}

bool CoreWin32Platform::ProcessMouseMoveEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (wParam != RIM_INPUT)
    {
        return false;
    }

    HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);
    RAWINPUT input;
    UINT dwSize = sizeof(input);

    GetRawInputData(hRawInput, RID_INPUT, &input, &dwSize, sizeof(RAWINPUTHEADER));

    if (input.header.dwType == RIM_TYPEMOUSE && input.data.mouse.usFlags == 0)
    {
        LONG xPos = input.data.mouse.lLastX;
        LONG yPos = input.data.mouse.lLastY;

        bool isMove = xPos || yPos;
        bool isInside = false;

        if (InputSystem::Instance()->GetMouseDevice().IsPinningEnabled())
        {
            isInside = true;
        }
        else
        {
            POINT pnt;
            GetCursorPos(&pnt);
            ScreenToClient(hWnd, &pnt);

            xPos += pnt.x;
            yPos += pnt.y;

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            isInside = xPos > clientRect.left &&
            xPos < clientRect.right &&
            yPos > clientRect.top &&
            yPos < clientRect.bottom;
        }

        if (isInside)
        {
            bool isMouseWheelChanged = (input.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) != 0;
            bool isMouseButtonsChanged = !isMouseWheelChanged &&
            (input.data.mouse.usButtonFlags >= RI_MOUSE_BUTTON_1_DOWN);

            if (isMove && !isMouseButtonsChanged)
            {
                OnMouseMove(xPos, yPos);
            }
        }
    }
    return false;
}

bool CoreWin32Platform::ProcessMouseWheelEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT pos;
    pos.x = GET_X_LPARAM(lParam);
    pos.y = GET_Y_LPARAM(lParam);
    short zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

    if (::ScreenToClient(hWnd, &pos) == FALSE || !IsCursorPointInside(hWnd, pos.x, pos.y))
    {
        return false;
    }

    if (message == WM_MOUSEHWHEEL)
    {
        OnMouseWheel(zDelta, 0, pos.x, pos.y);
    }
    else
    {
        OnMouseWheel(0, zDelta, pos.x, pos.y);
    }

    return true;
}

bool CoreWin32Platform::ProcessMouseInputEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (IsMouseClickEvent(message))
    {
        return ProcessMouseClickEvent(hWnd, message, wParam, lParam);
    }

    if (IsMouseMoveEvent(message))
    {
        return ProcessMouseMoveEvent(hWnd, message, wParam, lParam);
    }

    if (IsMouseWheelEvent(message))
    {
        return ProcessMouseWheelEvent(hWnd, message, wParam, lParam);
    }

    return false;
}

uint32 GetKeyboardModifiers()
{
    uint32 result = 0;
    static BYTE keys[256];
    memset(keys, 0, sizeof(keys));

    if (GetKeyboardState(keys))
    {
        if ((keys[VK_LSHIFT] >> 7) || (keys[VK_RSHIFT] >> 7))
        {
            result |= UIEvent::Modifier::SHIFT_DOWN;
        }
        if ((keys[VK_LCONTROL] >> 7) || (keys[VK_RCONTROL] >> 7))
        {
            result |= UIEvent::Modifier::CONTROL_DOWN;
        }
        if ((keys[VK_LMENU] >> 7) || (keys[VK_RMENU] >> 7))
        {
            result |= UIEvent::Modifier::ALT_DOWN;
        }
    }

    return result;
}

LRESULT CALLBACK CoreWin32Platform::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const UINT WM_ACTIVATE_POSTED = WM_USER + 12;

    CoreWin32Platform* core = static_cast<CoreWin32Platform*>(Core::Instance());
    RECT rect;

    // win32 app don't have ui-scaling option,
    // so hard-code default
    float32 uiScale = 1.0;

    if (IsMouseInputEvent(message, GetMessageExtraInfo()))
    {
        bool interruptProcessing = core->ProcessMouseInputEvent(hWnd, message, wParam, lParam);
        if (interruptProcessing)
        {
            return 0;
        }
    }
    if (message == MSG_ALREADY_RUNNING)
    {
        return MSG_ALREADY_RUNNING;
    }
    switch (message)
    {
    case WM_DPICHANGED:
        core->ApplyWindowSize();
        break;
    case WM_SIZE:
        GetClientRect(hWnd, &rect);
        core->WindowSizeChanged(static_cast<float32>(rect.right), static_cast<float32>(rect.bottom), uiScale, uiScale);
        break;
    case WM_ERASEBKGND:
        return 1; // https://msdn.microsoft.com/en-us/library/windows/desktop/ms648055%28v=vs.85%29.aspx
    case WM_SYSKEYUP:
    // no break
    case WM_KEYUP:
    {
        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

        uint32 systemKeyCode = static_cast<uint32>(wParam);
        uint32 extendedKeyInfo = static_cast<uint32>(lParam);
        if ((1 << 24) & extendedKeyInfo)
        {
            systemKeyCode |= 0x100;
        }
        uint32 scanCode = (extendedKeyInfo & 0xFF0000) >> 16;
        if (VK_SHIFT == systemKeyCode && scanCode == 0x36) // is right shift key
        {
            systemKeyCode |= 0x100;
        }

        UIEvent ev;
        ev.phase = UIEvent::Phase::KEY_UP;
        ev.key = keyboard.GetDavaKeyForSystemKey(systemKeyCode);
        ev.device = eInputDevices::KEYBOARD;
        ev.timestamp = (SystemTimer::GetMs() / 1000.0);
        ev.modifiers = GetKeyboardModifiers();

        UIControlSystem::Instance()->OnInput(&ev);
        keyboard.OnKeyUnpressed(ev.key);
        // Do not pass message to DefWindowProc to prevent system from sending WM_SYSCOMMAND when Alt is pressed
        return 0;
    }
    case WM_SYSKEYDOWN:
    // no break;
    case WM_KEYDOWN:
    {
        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

        uint32 systemKeyCode = static_cast<uint32>(wParam);
        uint32 extendedKeyInfo = static_cast<uint32>(lParam);
        if ((1 << 24) & extendedKeyInfo)
        {
            systemKeyCode |= 0x100;
        }
        uint32 scanCode = (extendedKeyInfo & 0xFF0000) >> 16;
        if (VK_SHIFT == systemKeyCode && scanCode == 0x36) // is right shift key
        {
            systemKeyCode |= 0x100;
        }

        UIEvent ev;
        if ((HIWORD(lParam) & KF_REPEAT) == 0)
        {
            ev.phase = UIEvent::Phase::KEY_DOWN;
        }
        else
        {
            ev.phase = UIEvent::Phase::KEY_DOWN_REPEAT;
        }
        ev.key = keyboard.GetDavaKeyForSystemKey(systemKeyCode);
        ev.device = eInputDevices::KEYBOARD;
        ev.timestamp = (SystemTimer::GetMs() / 1000.0);
        ev.modifiers = GetKeyboardModifiers();

        UIControlSystem::Instance()->OnInput(&ev);

        keyboard.OnKeyPressed(ev.key);
    };
    break;

    case WM_CHAR:
    {
        UIEvent ev;
        ev.keyChar = static_cast<char32_t>(wParam);
        if ((HIWORD(lParam) & KF_REPEAT) == 0)
        {
            ev.phase = UIEvent::Phase::CHAR;
        }
        else
        {
            ev.phase = UIEvent::Phase::CHAR_REPEAT;
        }
        ev.device = eInputDevices::KEYBOARD;
        ev.timestamp = (SystemTimer::GetMs() / 1000.0);
        ev.modifiers = GetKeyboardModifiers();

        UIControlSystem::Instance()->OnInput(&ev);
    }
    break;

    case WM_TOUCH:
    {
        UINT num_inputs = LOWORD(wParam);
        core->inputTouchBuffer.resize(num_inputs);
        PTOUCHINPUT inputs = core->inputTouchBuffer.data();
        HTOUCHINPUT h_touch_input = reinterpret_cast<HTOUCHINPUT>(lParam);

        if (GetTouchInputInfo(h_touch_input, num_inputs, inputs, sizeof(TOUCHINPUT)))
        {
            RECT rect;
            if (!GetClientRect(hWnd, &rect) ||
                (rect.right == rect.left && rect.bottom == rect.top))
            {
                break;
            }
            ClientToScreen(hWnd, reinterpret_cast<LPPOINT>(&rect));
            rect.top *= 100;
            rect.left *= 100;

            for (TOUCHINPUT& input : core->inputTouchBuffer)
            {
                float x_pixel = (input.x - rect.left) / 100.f;
                float y_pixel = (input.y - rect.top) / 100.f;

                if (input.dwFlags & TOUCHEVENTF_DOWN)
                {
                    core->OnTouchEvent(UIEvent::Phase::BEGAN, eInputDevices::TOUCH_SURFACE, input.dwID, x_pixel, y_pixel, 1.0f);
                }
                else if (input.dwFlags & TOUCHEVENTF_MOVE)
                {
                    core->OnTouchEvent(UIEvent::Phase::DRAG, eInputDevices::TOUCH_SURFACE, input.dwID, x_pixel, y_pixel, 1.0f);
                }
                else if (input.dwFlags & TOUCHEVENTF_UP)
                {
                    core->OnTouchEvent(UIEvent::Phase::ENDED, eInputDevices::TOUCH_SURFACE, input.dwID, x_pixel, y_pixel, 1.0f);
                }
            }
        }

        CloseTouchInputHandle(h_touch_input);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NCACTIVATE:
    {
        // Workaround for cases when WM_ACTIVATE not sent by system if main thread is busy
        // Example: resize window (forcing rhi to reset) -> press ctrl+alt+delete and unpress both ctrl and alt in system window
        // WM_ACTIVATE won't be sent, WM_KEYDOWN for ctrl and alt will be sent without according WM_KEYUP thus making KeyboardDevice think they're still pressed
        // But WM_NCACTIVATE will be sent and we can use it to clear keyboard state
        InputSystem* inputSystem = InputSystem::Instance();
        if (inputSystem != nullptr)
        {
            inputSystem->GetKeyboard().ClearAllKeys();
        }
        break;
    }

    case WM_ACTIVATE:
        // What dava.engine does when app is launched in fullscreen:
        //  1. create window in 'window' mode
        //  2. set window properties for fullscreen calling SetWindowPos API function
        //  3. SetWindowPos directly calls window procedure with WM_ACTIVATE message
        //  4. in WM_ACTIVATE handler dava.engine calls ApplicationCore::OnResume method before ApplicationCore::OnAppStarted
        //  5. kaboom - game crashes as game objects are null because they are created in ApplicationCore::OnAppStarted
        //
        // To resolve this problem simply post WM_ACTIVATE through system message queue
        PostMessage(hWnd, WM_ACTIVATE_POSTED, wParam, lParam);
        return 0;
    case WM_ACTIVATE_POSTED:
    {
        ApplicationCore* appCore = core->GetApplicationCore();

        int32 activationType = LOWORD(wParam);
        bool isMinimized = HIWORD(wParam) != 0;
        if (activationType == WA_INACTIVE)
        {
            Logger::FrameworkDebug("[PlatformWin32] deactivate application");
            Core::Instance()->FocusLost();
            EnableHighResolutionTimer(false);

            if (appCore)
            {
                // unpress all pressed buttons
                core->ClearMouseButtons();
                appCore->OnSuspend();
            }
            else
            {
                core->SetIsActive(false);
            }
        }
        else
        {
            if (isMinimized)
            {
                // Force set keyboard focus if window is activated from minimized state
                // See WM_ACTIVATE on MSDN: https://msdn.microsoft.com/en-us/library/windows/desktop/ms646274(v=vs.85).aspx
                SetFocus(hWnd);
            }

            Logger::FrameworkDebug("[PlatformWin32] activate application");

            //We need to activate high-resolution timer
            //cause default system timer resolution is ~15ms and our frame-time calculation is very inaccurate
            EnableHighResolutionTimer(true);

            Core::Instance()->FocusReceived();
            if (appCore)
            {
                appCore->OnResume();
            }
            else
            {
                core->SetIsActive(true);
            }
        }
        return 0;
    }
    case WM_SYSCOMMAND:
        // prevent screensaver or monitor powersave mode from starting
        if (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)
            return 0;
        break;
    case WM_GETMINMAXINFO:
        core->OnGetMinMaxInfo(reinterpret_cast<MINMAXINFO*>(lParam));
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CoreWin32Platform::InitArgs()
{
    SetCommandLine(GetCommandLineArgs());
}

void CoreWin32Platform::Quit()
{
    PostQuitMessage(0);
}

void* CoreWin32Platform::GetNativeWindow() const
{
    return hWindow;
}

bool AlreadyRunning()
{
    TCHAR szFileNameWithPath[MAX_PATH];
    TCHAR fileName[MAX_PATH];
    GetModuleFileName(NULL, szFileNameWithPath, MAX_PATH);
    _wsplitpath(szFileNameWithPath, nullptr, nullptr, fileName, nullptr);
    static const wchar_t* UID_ALREADY_RUNNING = L"_ALREADY-RUNNING-E34A9C2B-1894-4213-A280-7589641664CD";
    WideString mutexName = WideString(fileName) + WideString(UID_ALREADY_RUNNING);
    HANDLE hMutexOneInstance = ::CreateMutex(nullptr, FALSE, mutexName.c_str());
    // return ERROR_ACCESS_DENIED, if mutex created
    // in other session, as SECURITY_ATTRIBUTES == NULL
    return (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED);
}

void ShowRunningApplication()
{
    HWND hOther = nullptr;
    auto enumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        ULONG_PTR result;
        LRESULT ok = ::SendMessageTimeout(hWnd, MSG_ALREADY_RUNNING, 0, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 200, &result);
        if (ok == 0)
        {
            return true; // ignore & continue
        }
        if (result == MSG_ALREADY_RUNNING)
        {
            HWND* target = reinterpret_cast<HWND*>(lParam);
            *target = hWnd;
            return false; // if find
        }
        return true; // continue
    };
    EnumWindows(enumWindowsCallback, (LPARAM)&hOther);
    if (hOther != nullptr)
    {
        ::SetForegroundWindow(hOther);
        if (IsIconic(hOther))
        {
            ::ShowWindow(hOther, SW_RESTORE);
        }
    }
}

} // namespace DAVA
#endif // #if defined(__DAVAENGINE_WIN32__)
#endif // !__DAVAENGINE_COREV2__
