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

#include "Concurrency/Thread.h"
#include "Input/KeyboardDevice.h"
#include "Input/InputSystem.h"
#include "Platform/DeviceInfo.h"
#include "Platform/TemplateWin32/CorePlatformWin32.h"
#include "Platform/SystemTimer.h"
#include "Render/RenderManager.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA 
{
	int Core::Run(int argc, char * argv[], AppHandle handle)
	{
		CoreWin32Platform* core = new CoreWin32Platform();
        core->InitArgs();
        core->CreateSingletons();

        bool windowCreated = core->CreateWin32Window(handle);
		if(windowCreated)
		{
			core->Run();
			core->ReleaseSingletons();
			
		}

		return 0;
	
	}

	int Core::RunCmdTool(int argc, char * argv[], AppHandle handle)
	{
		CoreWin32Platform * core = new CoreWin32Platform();
        core->InitArgs();

		core->EnableConsoleMode();
		core->CreateSingletons();

        Logger::Instance()->EnableConsoleMode();
		
		FrameworkDidLaunched();
		FrameworkWillTerminate();
		core->ReleaseSingletons();
		return 0;
	}

	bool CoreWin32Platform::CreateWin32Window(HINSTANCE hInstance)
	{	
		this->hInstance = hInstance;

		//single instance check
		TCHAR fileName[MAX_PATH];
		GetModuleFileName(NULL, fileName, MAX_PATH);
		fileName[MAX_PATH-1] = 0; //string can be not null-terminated on winXP
		for(int32 i = 0; i < MAX_PATH; ++i)
		{
			if(fileName[i] == L'\\') //symbol \ is not allowed in CreateMutex mutex name
			{
				fileName[i] = ' ';
			}
		}
        SetLastError(0);
		//hMutex = CreateMutex(NULL, FALSE, fileName);
		//if(ERROR_ALREADY_EXISTS == GetLastError())
		//{
		//	return false;
		//}

		windowedMode = DisplayMode(800, 600, 16, 0);
		fullscreenMode = DisplayMode(800, 600, 16, 0);
		currentMode = windowedMode;
		isFullscreen = false;

		// create the window, only if we do not use the null device
		LPCWSTR className = L"DavaFrameworkWindowsDevice";

		// Register Class

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX); 
		wcex.style			= CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= 0;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= className;
		wcex.hIconSm		= 0;

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
			style  = WS_VISIBLE | WS_POPUP;
		} // End if Fullscreen


		AdjustWindowRect(&clientSize, style, FALSE);

		int32 realWidth = clientSize.right - clientSize.left;
		int32 realHeight = clientSize.bottom - clientSize.top;

		int32 windowLeft = -10000;//(GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
		int32 windowTop = -10000;//(GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

		if (isFullscreen)
		{
			windowLeft = 0;
			windowTop = 0;
		}

		// create window
		hWindow = CreateWindow( className, L"", style, windowLeft, windowTop, 
			realWidth, realHeight,	NULL, NULL, hInstance, NULL);

		ShowWindow(hWindow, SW_SHOW);
		UpdateWindow(hWindow);

		// fix ugly ATI driver bugs. Thanks to ariaci (Taken from Irrlight).
		MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);
	
#if defined(__DAVAENGINE_DIRECTX9__)
		RenderManager::Create(Core::RENDERER_DIRECTX9);
#elif defined(__DAVAENGINE_OPENGL__)
		RenderManager::Create(Core::RENDERER_OPENGL);
#endif
		RenderManager::Instance()->Create(hInstance, hWindow);
        RenderSystem2D::Instance()->Init();

		FrameworkDidLaunched();
		KeyedArchive * options = Core::GetOptions();

		fullscreenMode = GetCurrentDisplayMode();//FindBestMode(fullscreenMode);
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

			isFullscreen = (0 != options->GetInt32("fullscreen"));	
			String title = options->GetString("title", "[set application title using core options property 'title']");
			WideString titleW = StringToWString(title);
			SetWindowText(hWindow, titleW.c_str());
		}

		Logger::FrameworkDebug("[PlatformWin32] best display fullscreen mode matched: %d x %d x %d refreshRate: %d", fullscreenMode.width, fullscreenMode.height, fullscreenMode.bpp, fullscreenMode.refreshRate);

		currentMode = windowedMode;
		if (isFullscreen)
		{
			currentMode = fullscreenMode;
		}

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

        RenderManager::Instance()->ChangeDisplayMode(currentMode, isFullscreen);
        RenderManager::Instance()->Init(currentMode.width, currentMode.height);
        VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
        VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);

        isRightButtonPressed = false;
        isLeftButtonPressed = false;
        isMiddleButtonPressed = false;

		return true;
	}

	void CoreWin32Platform::Run()
	{
		Core::Instance()->SystemAppStarted();

		MSG msg;
		while(1)
		{
            DAVA::uint64 startTime = DAVA::SystemTimer::Instance()->AbsoluteMS();

			// process messages
			willQuit = false;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if(msg.message == WM_QUIT)
				{
					ApplicationCore * appCore = Core::Instance()->GetApplicationCore();
					if(appCore && appCore->OnQuit())
					{
						exit(0);
					}
					else
					{
						willQuit = true;
					}
				}
			}

            RenderManager::Instance()->Lock();
			Core::SystemProcessFrame();
			RenderManager::Instance()->Unlock();

            uint32 elapsedTime = (uint32) (SystemTimer::Instance()->AbsoluteMS() - startTime);
            int32 sleepMs = 1;

            int32 fps = RenderManager::Instance()->GetFPS();
            if(fps > 0)
            {
                sleepMs = (1000 / fps) - elapsedTime;
                if(sleepMs < 1)
                {
                    sleepMs = 1;
                }
            }

            Sleep(sleepMs);

			if (willQuit)
			{	
				break;
			}
		}

		Core::Instance()->SystemAppFinished();
		FrameworkWillTerminate();
	}

	RECT CoreWin32Platform::GetWindowedRectForDisplayMode(DisplayMode & dm)
	{
		RECT clientSize;
		clientSize.top = 0;
		clientSize.left = 0;
		clientSize.right = dm.width;
		clientSize.bottom = dm.height;

		AdjustWindowRect(&clientSize, GetWindowLong(hWindow, GWL_STYLE), FALSE);

		return clientSize;
	}

	void CoreWin32Platform::ToggleFullscreen()
	{
		// Setup styles based on windowed / fullscreen mode
		isFullscreen = !isFullscreen;

		if ( isFullscreen )
		{
			currentMode = fullscreenMode;
			GetWindowRect(hWindow, &windowPositionBeforeFullscreen);

			SetMenu( hWindow, NULL );
			SetWindowLong( hWindow, GWL_STYLE, FULLSCREEN_STYLE );
			SetWindowPos( hWindow, NULL, 0, 0, currentMode.width, currentMode.height, SWP_NOZORDER );
		} 
		else
		{
			SetWindowLong( hWindow, GWL_STYLE, WINDOWED_STYLE );

			currentMode = windowedMode;
			RECT windowedRect = GetWindowedRectForDisplayMode(currentMode);
	
			SetWindowPos( hWindow, HWND_NOTOPMOST, windowPositionBeforeFullscreen.left, windowPositionBeforeFullscreen.top, windowedRect.right - windowedRect.left, windowedRect.bottom - windowedRect.top, SWP_NOACTIVATE | SWP_SHOWWINDOW );
		}
		
		Logger::FrameworkDebug("[RenderManagerDX9] toggle mode: %d x %d isFullscreen: %d", currentMode.width, currentMode.height, isFullscreen);

		RenderManager::Instance()->ChangeDisplayMode(currentMode, isFullscreen);
		RenderManager::Instance()->Init(currentMode.width, currentMode.height);
        VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
        VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);
	}

	Core::eScreenMode CoreWin32Platform::GetScreenMode()
	{
		if (isFullscreen)return Core::MODE_FULLSCREEN;
		else return Core::MODE_WINDOWED;
	}

	void CoreWin32Platform::SwitchScreenToMode(eScreenMode screenMode)
	{
		if (GetScreenMode() != screenMode) // check if we try to switch mode
		{
			if (screenMode == Core::MODE_FULLSCREEN)
			{
				ToggleFullscreen();
			}else if (screenMode == Core::MODE_WINDOWED)
			{
				ToggleFullscreen();
			}
		}else
		{
		}
	}

	void CoreWin32Platform::GetAvailableDisplayModes(List<DisplayMode> & availableDisplayModes)
	{
		availableDisplayModes.clear();

		DWORD iModeNum = 0;
		DEVMODE	dmi;
		ZeroMemory (&dmi, sizeof(dmi)) ;
		dmi.dmSize = sizeof(dmi) ;

		while(EnumDisplaySettings(NULL, iModeNum++, &dmi))
		{
			DisplayMode mode;
			mode.width = dmi.dmPelsWidth;
			mode.height = dmi.dmPelsHeight;
			mode.bpp = dmi.dmBitsPerPel;
			mode.refreshRate = dmi.dmDisplayFrequency;
			ZeroMemory (&dmi, sizeof(dmi)) ;
			availableDisplayModes.push_back(mode);
		}
	}

	DisplayMode CoreWin32Platform::GetCurrentDisplayMode()
	{
		DWORD iModeNum = 0;
		DEVMODE	dmi;
		ZeroMemory (&dmi, sizeof(dmi)) ;
		dmi.dmSize = sizeof(dmi);

		DisplayMode mode;
		if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmi))
		{
			mode.width = dmi.dmPelsWidth;
			mode.height = dmi.dmPelsHeight;
			mode.bpp = dmi.dmBitsPerPel;
			mode.refreshRate = dmi.dmDisplayFrequency;
			ZeroMemory (&dmi, sizeof(dmi)) ;
		}

		return mode;
	}

	void CoreWin32Platform::SetIcon(int32 iconId)
	{
		HINSTANCE hInst= GetModuleHandle(0);
		HICON smallIcon = static_cast<HICON>(LoadImage(hInst,
			MAKEINTRESOURCE(iconId),
			IMAGE_ICON,
			0,
			0,
			LR_DEFAULTSIZE));
		SendMessage(hWindow, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
		SendMessage(hWindow, WM_SETICON, ICON_BIG, (LPARAM)smallIcon);
	}

    int32 CoreWin32Platform::MoveTouchsToVector(UIEvent::PointerDeviceID deviceId, USHORT buttsFlags, WPARAM wParam, LPARAM lParam, Vector<UIEvent>* outTouches)
    {
        int button = 0;
        int phase = -1;

        if(LOWORD(wParam))
            phase = UIEvent::PHASE_MOVE;

        if(isLeftButtonPressed)
            button = 1;
        else if(isRightButtonPressed)
            button = 2;
        else if(isMiddleButtonPressed)
            button = 3;

		if(buttsFlags & RI_MOUSE_LEFT_BUTTON_DOWN || buttsFlags & RI_MOUSE_RIGHT_BUTTON_DOWN || buttsFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
		{
            phase = UIEvent::PHASE_BEGAN;
            if(buttsFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
            {
                isLeftButtonPressed = true;
                button = 1;
            }
            if(buttsFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
            {
                isRightButtonPressed = true;
                button = 2;
            }
            if(buttsFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
            {
                isMiddleButtonPressed = true;
                button = 3;
            }
		}
		else if(buttsFlags & RI_MOUSE_LEFT_BUTTON_UP || buttsFlags & RI_MOUSE_RIGHT_BUTTON_UP || buttsFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
		{
            phase = UIEvent::PHASE_ENDED;
            if(buttsFlags & RI_MOUSE_LEFT_BUTTON_UP)
            {
                isLeftButtonPressed = false;
                button = 1;
            }
            if(buttsFlags & RI_MOUSE_RIGHT_BUTTON_UP)
            {
                isRightButtonPressed = false;
                button = 2;
            }
            if(buttsFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
            {
                isMiddleButtonPressed = false;
                button = 3;
            }
		}
		else if(button && phase == UIEvent::PHASE_MOVE)
		{
			phase = UIEvent::PHASE_DRAG;
		}

        if(phase == -1)
            return phase;

		bool isFind = false;
        for (Vector<DAVA::UIEvent>::iterator it = events.begin(); it != events.end(); it++)
        {
            if (it->tid == button)
            {
                isFind = true;

                it->physPoint.x = static_cast<float32>(GET_X_LPARAM(lParam));
                it->physPoint.y = static_cast<float32>(GET_Y_LPARAM(lParam));
                it->phase = phase;

                break;
            }
        }

		if(!isFind)
		{
			UIEvent newTouch;
			newTouch.tid = button;
            newTouch.physPoint.x = static_cast<float32>(GET_X_LPARAM(lParam));
            newTouch.physPoint.y = static_cast<float32>(GET_Y_LPARAM(lParam));
            newTouch.phase = phase;
            newTouch.deviceId = deviceId;
            events.push_back(newTouch);
        }

        for (Vector<DAVA::UIEvent>::iterator it = events.begin(); it != events.end(); it++)
        {
            outTouches->push_back(*it);
        }

        if(phase == UIEvent::PHASE_ENDED || phase == UIEvent::PHASE_MOVE)
		{
            for (Vector<DAVA::UIEvent>::iterator it = events.begin(); it != events.end(); it++)
            {
                if (it->tid == button)
                {
                    events.erase(it);
                    break;
                }
            }
        }

		return phase;
	}

	static bool mouseCursorShown = true;
	static USHORT mouseButtonsDownMask = 0;

	void HandleMouseButtonsPressed(USHORT buttsFlags)
	{
		if (buttsFlags & RI_MOUSE_BUTTON_1_DOWN)
		{
			mouseButtonsDownMask |= RI_MOUSE_BUTTON_1_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_2_DOWN)
		{
			mouseButtonsDownMask |= RI_MOUSE_BUTTON_2_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_3_DOWN)
		{
			mouseButtonsDownMask |= RI_MOUSE_BUTTON_3_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_4_DOWN)
		{
			mouseButtonsDownMask |= RI_MOUSE_BUTTON_4_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_5_DOWN)
		{
			mouseButtonsDownMask |= RI_MOUSE_BUTTON_5_DOWN;
		}
	}

	void HandleMouseButtonsReleased(USHORT buttsFlags)
	{
		if (mouseButtonsDownMask == 0)
		{
			return;
		}

		// Reset the mouse buttons mask, release capture if mask is empty (all buttons released).
		if (buttsFlags & RI_MOUSE_BUTTON_1_UP)
		{
			mouseButtonsDownMask &= ~RI_MOUSE_BUTTON_1_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_2_UP)
		{
			mouseButtonsDownMask &= ~RI_MOUSE_BUTTON_2_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_3_UP)
		{
			mouseButtonsDownMask &= ~RI_MOUSE_BUTTON_3_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_4_UP)
		{
			mouseButtonsDownMask &= ~RI_MOUSE_BUTTON_4_DOWN;
		}
		if (buttsFlags & RI_MOUSE_BUTTON_5_UP)
		{
			mouseButtonsDownMask &= ~RI_MOUSE_BUTTON_5_DOWN;
		}
	}

    void CoreWin32Platform::OnMouseEvent(UIEvent::PointerDeviceID deviceId, USHORT buttsFlags, WPARAM wParam, LPARAM lParam, USHORT buttonData)
    {
        Vector<DAVA::UIEvent> touches;
        int32 touchPhase = -1;

        if (HIWORD(wParam) || mouseButtonsDownMask > 0) // isPoint inside window or some clicks already captured
        {
            HandleMouseButtonsPressed(buttsFlags);
        }

        if(buttsFlags & RI_MOUSE_WHEEL)
        {
            touchPhase = UIEvent::PHASE_WHEEL;

            UIEvent newTouch;
            newTouch.tid = 0;
            newTouch.physPoint.x = 0;
            newTouch.physPoint.y = ((SHORT)buttonData) / (float32)(WHEEL_DELTA);
            newTouch.phase = UIEvent::PHASE_WHEEL;
            newTouch.deviceId = deviceId;

            touches.push_back(newTouch);
        }
        else
		{
            if(HIWORD(wParam) || mouseButtonsDownMask > 0) // HIWORD(wParam) - isPoint inside window
			{
                touchPhase = MoveTouchsToVector(deviceId, buttsFlags, wParam, lParam, &touches);
            }
        }

        if (touchPhase != -1)
        {
            UIControlSystem::Instance()->OnInput(touches, events);
        }

        if (RenderManager::Instance()->GetCursor() != nullptr && mouseCursorShown)
        {
            ShowCursor(false);
            mouseCursorShown = false;
        }
        if (RenderManager::Instance()->GetCursor() == nullptr && !mouseCursorShown)
        {
            ShowCursor(false);
            mouseCursorShown = false;
        }

		HandleMouseButtonsReleased(buttsFlags);
	}

    void CoreWin32Platform::OnTouchEvent(UIEvent::eInputPhase phase, UIEvent::PointerDeviceID deviceId, uint32 fingerId, float32 x, float32 y, float presure)
    {
        UIEvent newTouch;
        newTouch.tid = 0;
        newTouch.physPoint.x = x;
        newTouch.physPoint.y = y;
        newTouch.phase = phase;
        newTouch.deviceId = deviceId;

        Vector<UIEvent> touches;
        touches.push_back(newTouch);

        UIControlSystem::Instance()->OnInput(touches, events);
    }

    struct MouseDevice
    {
        uint32 which; // mouse index
        String name; // name for debug
    };

    struct TouchDevice
    {
        uint32 which; // surface index
        String name; // name for debug
    };

    String CoreWin32Platform::GetDeviceName(HANDLE hDevice)
    {
        std::array<char, 1024> buffer;
        UINT size = static_cast<UINT>(buffer.size());
        int resultSize = GetRawInputDeviceInfoA(hDevice, RIDI_DEVICENAME, &buffer[0], &size);
        if (resultSize > 0)
        {
            return String(buffer.data(), static_cast<size_t>(resultSize));
        }

        DVASSERT(false && "Failed to get device name");
        return String();
    }

    LRESULT CALLBACK CoreWin32Platform::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        CoreWin32Platform* core = static_cast<CoreWin32Platform*>(Core::Instance());
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA                     
#define WHEEL_DELTA 120
#endif

        KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

        switch (message)
        {
        case WM_ERASEBKGND:
            return 1; // https://msdn.microsoft.com/en-us/library/windows/desktop/ms648055%28v=vs.85%29.aspx
        case WM_KEYUP:
        {
            DAVA::UIEvent ev;
            ev.keyChar = 0;
            ev.phase = DAVA::UIEvent::PHASE_KEYCHAR_RELEASE;
            ev.tapCount = 1;

            int32 system_key_code = static_cast<int32>(wParam);
            ev.tid = keyboard.GetDavaKeyForSystemKey(system_key_code);

            Vector<DAVA::UIEvent> touches;
            touches.push_back(ev);

            UIControlSystem::Instance()->OnInput(touches, core->events);

            keyboard.OnSystemKeyUnpressed(system_key_code);
        }
        break;

        case WM_KEYDOWN:
        {
            DAVA::UIEvent ev;
            ev.keyChar = 0;
            ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
            ev.tapCount = 1;

            int32 system_key_code = static_cast<int32>(wParam);
            ev.tid = keyboard.GetDavaKeyForSystemKey(system_key_code);

            Vector<DAVA::UIEvent> touches;
            touches.push_back(ev);

            UIControlSystem::Instance()->OnInput(touches, core->events);

            keyboard.OnSystemKeyPressed(system_key_code);
        };
        break;

        case WM_CHAR:
        {
            DAVA::UIEvent ev;
            ev.keyChar = static_cast<char16>(wParam);
            ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
            ev.tapCount = 1;
            ev.tid = 0;

            Vector<DAVA::UIEvent> touches;
            touches.push_back(ev);

            UIControlSystem::Instance()->OnInput(touches, core->events);
        }
        break;

        case WM_INPUT:
        {
            HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);
            RAWINPUT inp;
            UINT dwSize = sizeof(inp);

            GetRawInputData(hRawInput, RID_INPUT, &inp, &dwSize, sizeof(RAWINPUTHEADER));

            if (inp.header.dwType == RIM_TYPEMOUSE && inp.data.mouse.usFlags == 0)
            {
                LONG x = inp.data.mouse.lLastX;
                LONG y = inp.data.mouse.lLastY;

                bool isMove = x || y;

                if(InputSystem::Instance()->IsCursorPining())
                {
                    SetCursorPosCenterInternal(hWnd);
                }
                else
                {
                    POINT p;
                    GetCursorPos(&p);
                    ScreenToClient(hWnd, &p);
                    x += p.x;
                    y += p.y;
                }

                RECT clientRect;
                GetClientRect(hWnd, &clientRect);

                bool isInside = (x > clientRect.left && x < clientRect.right && y > clientRect.top && y < clientRect.bottom) || InputSystem::Instance()->IsCursorPining();

                core->OnMouseEvent(UIEvent::PointerDeviceID::MOUSE, inp.data.mouse.usButtonFlags, MAKEWPARAM(isMove, isInside), MAKELPARAM(x, y), inp.data.mouse.usButtonData); // only move, drag and wheel events
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
                        core->OnTouchEvent(UIEvent::PHASE_BEGAN, UIEvent::PointerDeviceID::TOUCH, input.dwID, x_pixel, y_pixel, 1.0f);
                    }
                    else if (input.dwFlags & TOUCHEVENTF_MOVE)
                    {
                        core->OnTouchEvent(UIEvent::PHASE_MOVE, UIEvent::PointerDeviceID::TOUCH, input.dwID, x_pixel, y_pixel, 1.0f);
                    }
                    else if (input.dwFlags & TOUCHEVENTF_UP)
                    {
                        core->OnTouchEvent(UIEvent::PHASE_ENDED, UIEvent::PointerDeviceID::TOUCH, input.dwID, x_pixel, y_pixel, 1.0f);
                    }
                }
            }

            CloseTouchInputHandle(h_touch_input);
            return 0;
        }
        case WM_NCMOUSEMOVE:
            if (!mouseCursorShown)
            {
                ShowCursor(true);
                mouseCursorShown = true;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_ACTIVATE:
			{
				ApplicationCore * core = Core::Instance()->GetApplicationCore();
                WORD loWord = LOWORD(wParam);
                WORD hiWord = HIWORD(wParam);
                if(!loWord || hiWord)
                {
                    Logger::FrameworkDebug("[PlatformWin32] deactivate application");
                    RenderResource::SaveAllResourcesToSystemMem();
					
                    if(core)
					{
						core->OnSuspend();
					}
					else 
					{
						Core::Instance()->SetIsActive(false);
					}
                }
                else
                {
                    Logger::FrameworkDebug("[PlatformWin32] activate application");
					if(core)
					{
						core->OnResume();
					}
					else 
					{
						Core::Instance()->SetIsActive(true);
					}
                }
			};
			break;
		case WM_SYSCOMMAND:
			// prevent screensaver or monitor powersave mode from starting
			if (wParam == SC_SCREENSAVE ||
				wParam == SC_MONITORPOWER)
				return 0;
			break;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
#endif // #if defined(__DAVAENGINE_WIN32__)
