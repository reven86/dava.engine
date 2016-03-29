/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>
#include "Debug/Profiler.h"

#include "Concurrency/Thread.h"
#include "Input/KeyboardDevice.h"
#include "Input/InputSystem.h"
#include "Platform/DPIHelper.h"
#include "Platform/DeviceInfo.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"
#include "Platform/SystemTimer.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"

#include "MemoryManager/MemoryProfiler.h"

#include "Input/MouseCapture.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{
int Core::Run(int argc, char* argv[], AppHandle handle)
{
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
        style = WS_VISIBLE | WS_POPUP;
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
    HWND hWindow = CreateWindow(className, L"", style, windowLeft, windowTop,
                                realWidth, realHeight, NULL, NULL, hInstance, NULL);

    SetMenu(hWindow, NULL);

    // fix ugly ATI driver bugs. Thanks to ariaci (Taken from Irrlight).
    MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);

    FrameworkDidLaunched();
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

    if (options)
    {
        windowedMode.width = options->GetInt32("width");
        windowedMode.height = options->GetInt32("height");
        windowedMode.bpp = options->GetInt32("bpp");

        // get values from config in case if they are available
        fullscreenMode.width = options->GetInt32("fullscreen.width", fullscreenMode.width);
        fullscreenMode.height = options->GetInt32("fullscreen.height", fullscreenMode.height);
        fullscreenMode.bpp = windowedMode.bpp;

        fullscreenMode = FindBestMode(fullscreenMode);
        shouldEnableFullscreen = options->GetInt32("fullscreen", 0) == 1;
        String title = options->GetString("title", "[set application title using core options property 'title']");
        WideString titleW = StringToWString(title);
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

        windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
        windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

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

    VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
    VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);

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
        uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

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

        uint32 elapsedTime = (uint32)(SystemTimer::Instance()->AbsoluteMS() - startTime);
        int32 sleepMs = 1;

        int32 fps = Renderer::GetDesiredFPS();
        if (fps > 0)
        {
            sleepMs = (1000 / fps) - elapsedTime;
            if (sleepMs < 1)
            {
                sleepMs = 1;
            }
        }

        TRACE_BEGIN_EVENT(11, "core", "Sleep");
        Sleep(sleepMs);
        TRACE_END_EVENT(11, "core", "Sleep");

        if (willQuit)
        {
            break;
        }
    }

    Core::Instance()->SystemAppFinished();
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
    if (GetScreenMode() != screenMode) // check if we try to switch mode
    {
        HWND hWindow = static_cast<HWND>(GetNativeView());
        switch (screenMode)
        {
        case DAVA::Core::eScreenMode::FULLSCREEN:
        {
            isFullscreen = true;
            currentMode = fullscreenMode;
            GetWindowRect(hWindow, &windowPositionBeforeFullscreen);
            SetWindowLong(hWindow, GWL_STYLE, FULLSCREEN_STYLE);
            MONITORINFO monitor_info;
            monitor_info.cbSize = sizeof(monitor_info);
            GetMonitorInfo(MonitorFromWindow(hWindow, MONITOR_DEFAULTTONEAREST), &monitor_info);
            RECT window_rect(monitor_info.rcMonitor);

            SetWindowPos(hWindow, NULL, window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, SWP_NOZORDER);
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
            RECT windowedRect = GetWindowedRectForDisplayMode(currentMode);
            SetWindowPos(hWindow, HWND_NOTOPMOST, windowPositionBeforeFullscreen.left, windowPositionBeforeFullscreen.top, windowPositionBeforeFullscreen.right - windowPositionBeforeFullscreen.left, windowPositionBeforeFullscreen.bottom - windowPositionBeforeFullscreen.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
            break;
        }
        default:
        {
            DVASSERT_MSG(false, "Incorrect screen mode");
            Logger::Error("Incorrect screen mode");
            return false;
        }
        }

        Logger::FrameworkDebug("[RenderManagerDX9] toggle mode: %d x %d isFullscreen: %d", currentMode.width, currentMode.height, isFullscreen);
        VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
        VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);
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

struct MouseButtonChange
{
    UIEvent::MouseButton button;
    UIEvent::Phase beginOrEnd;
};

static Vector<MouseButtonChange> mouseButtonChanges;

void GetMouseButtonChanges(uint16_t mouseButtonFlags, Vector<MouseButtonChange>& out)
{
    out.clear();
    if (mouseButtonFlags & RI_MOUSE_BUTTON_1_UP)
    {
        out.push_back({ UIEvent::MouseButton::LEFT, UIEvent::Phase::ENDED });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_2_UP)
    {
        out.push_back({ UIEvent::MouseButton::RIGHT, UIEvent::Phase::ENDED });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_3_UP)
    {
        out.push_back({ UIEvent::MouseButton::MIDDLE, UIEvent::Phase::ENDED });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_4_UP)
    {
        out.push_back({ UIEvent::MouseButton::EXTENDED1, UIEvent::Phase::ENDED });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_5_UP)
    {
        out.push_back({ UIEvent::MouseButton::EXTENDED2, UIEvent::Phase::ENDED });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
    {
        out.push_back({ UIEvent::MouseButton::LEFT, UIEvent::Phase::BEGAN });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
    {
        out.push_back({ UIEvent::MouseButton::RIGHT, UIEvent::Phase::BEGAN });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
    {
        out.push_back({ UIEvent::MouseButton::MIDDLE, UIEvent::Phase::BEGAN });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
    {
        out.push_back({ UIEvent::MouseButton::EXTENDED1, UIEvent::Phase::BEGAN });
    }
    if (mouseButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
    {
        out.push_back({ UIEvent::MouseButton::EXTENDED2, UIEvent::Phase::BEGAN });
    }
}

void CoreWin32Platform::OnMouseMove(float32 x, float32 y)
{
    UIEvent e;
    e.physPoint = Vector2(x, y);
    e.device = UIEvent::Device::MOUSE;
    e.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

    if (mouseButtonState.any())
    {
        for (unsigned buttonIndex = static_cast<unsigned>(UIEvent::MouseButton::LEFT);
             buttonIndex <= static_cast<unsigned>(UIEvent::MouseButton::EXTENDED2);
             ++buttonIndex)
        {
            unsigned bitIndex = buttonIndex - 1;
            if (mouseButtonState[bitIndex])
            {
                e.mouseButton = static_cast<UIEvent::MouseButton>(buttonIndex);
                e.phase = UIEvent::Phase::DRAG;
                UIControlSystem::Instance()->OnInput(&e);
            }
        }
    }
    else
    {
        e.mouseButton = UIEvent::MouseButton::NONE;
        e.phase = UIEvent::Phase::MOVE;
        UIControlSystem::Instance()->OnInput(&e);
    }
}

void CoreWin32Platform::OnMouseWheel(float32 wheelDelta, float32 x, float32 y)
{
    UIEvent e;
    e.physPoint = Vector2(x, y);
    e.device = UIEvent::Device::MOUSE;
    e.phase = UIEvent::Phase::WHEEL;
    e.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

    KeyboardDevice& keybDev = InputSystem::Instance()->GetKeyboard();
    if (keybDev.IsKeyPressed(Key::LSHIFT) || keybDev.IsKeyPressed(Key::RSHIFT))
    {
        e.wheelDelta = { wheelDelta, 0 };
    }
    else
    {
        e.wheelDelta = { 0, wheelDelta };
    }

    UIControlSystem::Instance()->OnInput(&e);
}

void CoreWin32Platform::OnMouseButtonChange(UIEvent::Phase phase, UIEvent::MouseButton button, float32 x, float32 y)
{
    UIEvent e;
    e.physPoint = Vector2(x, y);
    e.device = UIEvent::Device::MOUSE;
    e.phase = phase;
    e.mouseButton = button;
    e.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

    UIControlSystem::Instance()->OnInput(&e);
}

void CoreWin32Platform::OnTouchEvent(UIEvent::Phase phase, UIEvent::Device deviceId, uint32 fingerId, float32 x, float32 y, float presure)
{
    UIEvent newTouch;
    newTouch.touchId = fingerId;
    newTouch.physPoint.x = x;
    newTouch.physPoint.y = y;
    newTouch.phase = phase;
    newTouch.device = deviceId;
    newTouch.tapCount = 1;
    newTouch.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

    UIControlSystem::Instance()->OnInput(&newTouch);
}

void CoreWin32Platform::OnGetMinMaxInfo(MINMAXINFO* minmaxInfo)
{
    if (minWindowWidth > 0.0f && minWindowHeight > 0.0f)
    {
        minmaxInfo->ptMinTrackSize.x = static_cast<LONG>(minWindowWidth);
        minmaxInfo->ptMinTrackSize.y = static_cast<LONG>(minWindowHeight);
    }
}

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

LRESULT CALLBACK CoreWin32Platform::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CoreWin32Platform* core = static_cast<CoreWin32Platform*>(Core::Instance());
    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    //TODO: Add system scale
    float32 scaleX = 1.f;
    float32 scaleY = 1.f;
    scaleX = scaleY = static_cast<float32>(DPIHelper::GetDpiScaleFactor(0));
    RECT rect;

    switch (message)
    {
    case WM_SIZE:
        GetClientRect(hWnd, &rect);
        core->WindowSizeChanged(static_cast<float32>(rect.right), static_cast<float32>(rect.bottom), scaleX, scaleY);
        break;
    case WM_ERASEBKGND:
        return 1; // https://msdn.microsoft.com/en-us/library/windows/desktop/ms648055%28v=vs.85%29.aspx
    case WM_SYSKEYUP:
    // no break
    case WM_KEYUP:
    {
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
        ev.device = UIEvent::Device::KEYBOARD;
        ev.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

        UIControlSystem::Instance()->OnInput(&ev);

        keyboard.OnKeyUnpressed(ev.key);
    }
    break;

    case WM_SYSKEYDOWN:
    // no break;
    case WM_KEYDOWN:
    {
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
        ev.device = UIEvent::Device::KEYBOARD;
        ev.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

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
        ev.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

        UIControlSystem::Instance()->OnInput(&ev);
    }
    break;

    case WM_INPUT:
    {
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms645590%28v=vs.85%29.aspx
        bool isApplicationInForeground = (wParam == 0);
        if (isApplicationInForeground)
        {
            HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);
            RAWINPUT input;
            UINT dwSize = sizeof(input);

            GetRawInputData(hRawInput, RID_INPUT, &input, &dwSize, sizeof(RAWINPUTHEADER));

            if (input.header.dwType == RIM_TYPEMOUSE && input.data.mouse.usFlags == 0)
            {
                LONG x = input.data.mouse.lLastX;
                LONG y = input.data.mouse.lLastY;

                bool isMove = x || y;
                bool isInside = false;

                POINT p;
                GetCursorPos(&p);
                ScreenToClient(hWnd, &p);

                if (MouseCapture::GetMouseCaptureModeNative() == InputSystem::eMouseCaptureMode::PINING)
                {
                    isInside = true;
                }
                else
                {
                    x += p.x;
                    y += p.y;

                    RECT clientRect;
                    GetClientRect(hWnd, &clientRect);
                    isInside = (x > clientRect.left && x < clientRect.right && y > clientRect.top && y < clientRect.bottom);
                }

                float32 x_pos = static_cast<float32>(x);
                float32 y_pos = static_cast<float32>(y);

                if (isInside)
                {
                    bool isMouseWheelChanged = (input.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) != 0;
                    bool isMouseButtonsChanged = (!isMouseWheelChanged && (input.data.mouse.usButtonFlags >= RI_MOUSE_BUTTON_1_DOWN));

                    if (isMove && !isMouseButtonsChanged)
                    {
                        core->OnMouseMove(x_pos, y_pos);
                    }
                    if (isMouseWheelChanged)
                    {
                        float32 wheelDelta = static_cast<int16>(input.data.mouse.usButtonData) / static_cast<float32>(WHEEL_DELTA);
                        core->OnMouseWheel(wheelDelta, x_pos, y_pos);
                    }
                    if (isMouseButtonsChanged)
                    {
                        GetMouseButtonChanges(input.data.mouse.usButtonFlags, mouseButtonChanges);
                        for (auto& change : mouseButtonChanges)
                        {
                            core->OnMouseButtonChange(change.beginOrEnd, change.button, static_cast<float32>(p.x), static_cast<float32>(p.y));
                            bool isButtonDown = (change.beginOrEnd == UIEvent::Phase::BEGAN);
                            unsigned buttonIndex = static_cast<unsigned>(change.button) - 1;
                            core->mouseButtonState[buttonIndex] = isButtonDown;
                        }
                    }
                    return 0;
                }
            }
        }
        break;
    }
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
                    core->OnTouchEvent(UIEvent::Phase::BEGAN, UIEvent::Device::TOUCH_SURFACE, input.dwID, x_pixel, y_pixel, 1.0f);
                }
                else if (input.dwFlags & TOUCHEVENTF_MOVE)
                {
                    core->OnTouchEvent(UIEvent::Phase::MOVE, UIEvent::Device::TOUCH_SURFACE, input.dwID, x_pixel, y_pixel, 1.0f);
                }
                else if (input.dwFlags & TOUCHEVENTF_UP)
                {
                    core->OnTouchEvent(UIEvent::Phase::ENDED, UIEvent::Device::TOUCH_SURFACE, input.dwID, x_pixel, y_pixel, 1.0f);
                }
            }
        }

        CloseTouchInputHandle(h_touch_input);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_ACTIVATE:
    {
        ApplicationCore* appCore = core->GetApplicationCore();
        WORD loWord = LOWORD(wParam);
        WORD hiWord = HIWORD(wParam);
        if (!loWord || hiWord)
        {
            Logger::FrameworkDebug("[PlatformWin32] deactivate application");

            if (appCore)
            {
                MouseCapture::SetApplicationFocus(false);
                appCore->OnSuspend();
            }
            else
            {
                core->SetIsActive(false);
            }
        }
        else
        {
            Logger::FrameworkDebug("[PlatformWin32] activate application");
            if (appCore)
            {
                MouseCapture::SetApplicationFocus(true);
                appCore->OnResume();
            }
            else
            {
                core->SetIsActive(true);
            }
        }
    };
    break;
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
    int argc = 0;
    LPWSTR* szArglist = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

    if (argc > 0 && NULL != szArglist)
    {
        Vector<String> args;
        args.reserve(argc);
        for (int i = 0; i < argc; ++i)
        {
            args.emplace_back(WStringToString(szArglist[i]));
        }

        SetCommandLine(std::move(args));
    }

    ::LocalFree(szArglist);
}

void CoreWin32Platform::Quit()
{
    PostQuitMessage(0);
}

} // namespace DAVA
#endif // #if defined(__DAVAENGINE_WIN32__)
