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


#include "Platform/TemplateWin32/CorePlatformWin32.h"
#include "Platform/TemplateWin32/WindowsSpecifics.h"
#include "Platform/Thread.h"
#include "Platform/DeviceInfo.h"
#include "Utils/Utils.h"

#if defined(__DAVAENGINE_WIN32__)

#pragma warning( disable: 7 9 193 271 304 791 )
#include <d3d9.h>
#pragma warning( default: 7 9 193 271 304 791 )

#include <shellapi.h>

#include "Render/RHI/DX9/_dx9.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

HDC deviceContext = 0;
HGLRC glRenderContext = 0;

D3DPRESENT_PARAMETERS d3dPresentParams;
IDirect3DDevice9 * d3d9Device = nullptr;
bool d3d9DeviceLost = false;

//------------------------------------------------------------------------------

#define E_MINSPEC (-3)  // Error code for gfx-card that doesn't meet min.spec

bool IsValidIntelCard(unsigned vendor_id, unsigned device_id)
{
    return ((vendor_id == 0x8086) &&  // Intel Architecture

        // These guys are prehistoric :)

        (device_id == 0x2572) ||  // 865G
        (device_id == 0x3582) ||  // 855GM
        (device_id == 0x2562) ||  // 845G
        (device_id == 0x3577) ||  // 830M

        // These are from 2005 and later

        (device_id == 0x2A02) ||  // GM965 Device 0 
        (device_id == 0x2A03) ||  // GM965 Device 1 
        (device_id == 0x29A2) ||  // G965 Device 0 
        (device_id == 0x29A3) ||  // G965 Device 1 
        (device_id == 0x27A2) ||  // 945GM Device 0 
        (device_id == 0x27A6) ||  // 945GM Device 1 
        (device_id == 0x2772) ||  // 945G Device 0 
        (device_id == 0x2776) ||  // 945G Device 1 
        (device_id == 0x2592) ||  // 915GM Device 0 
        (device_id == 0x2792) ||  // 915GM Device 1 
        (device_id == 0x2582) ||  // 915G Device 0 
        (device_id == 0x2782)     // 915G Device 1 
        );
}

//------------------------------------------------------------------------------

namespace DAVA 
{
	int Core::Run(int argc, char * argv[], AppHandle handle)
	{
		CoreWin32Platform * core = new CoreWin32Platform();
		core->CreateSingletons();
        core->InitArgs();

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

		//core->CreateWin32Window(handle);
		//core->Run();
		core->EnableConsoleMode();
		core->CreateSingletons();

		core->InitArgs();

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

        deviceContext = ::GetDC(hWindow);

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

        rhi::Api renderApi = (rhi::Api)options->GetInt32("renderer", rhi::RHI_DX9);
        switch (renderApi)
        {
        case rhi::RHI_DX9:
            CreateDirect3DDevice();
            break;
        case rhi::RHI_GLES2:
            CreateGLContext();
            break;
        default:
            DVASSERT(false);
            break;
        }

        RAWINPUTDEVICE Rid;

        Rid.usUsagePage = 0x01; 
        Rid.usUsage = 0x02; 
        Rid.dwFlags = 0;
        Rid.hwndTarget = 0;

        RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
				
        VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
        VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);

        isRightButtonPressed = false;
        isLeftButtonPressed = false;
        isMiddleButtonPressed = false;

		return true;
	}

    void EndFrameDX9()
    {
        HRESULT hr;

        if (d3d9DeviceLost)
        {
            hr = d3d9Device->TestCooperativeLevel();

            if (hr == D3DERR_DEVICENOTRESET)
            {
                d3d9DeviceLost = false;
            }
            else
            {
                ::Sleep(100);
            }
        }
        else
        {
            hr = d3d9Device->Present(NULL, NULL, NULL, NULL);

            if (FAILED(hr))
                Logger::Error("present() failed:\n%s\n", D3D9ErrorText(hr));

            if (hr == D3DERR_DEVICELOST)
                d3d9DeviceLost = true;
        }
    }

    void CoreWin32Platform::CreateDirect3DDevice()
    {
        IDirect3D9 * _D3D9 = Direct3DCreate9(D3D_SDK_VERSION);

        if (_D3D9)
        {
            RECT clientRect;
            GetClientRect(hWindow, &clientRect);

            HRESULT                 hr;
            unsigned                backbuf_width = clientRect.right - clientRect.left;
            unsigned                backbuf_height = clientRect.bottom - clientRect.top;
            bool                    use_vsync = true;//(vsync)  ? (bool)(*vsync)  : false;
            D3DADAPTER_IDENTIFIER9  info = { 0 };
            D3DCAPS9                caps;
            DWORD                   vertex_processing = E_FAIL;

            hr = _D3D9->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &info);


            // check if running on Intel card

            Logger::Info("vendor-id : %04X  device-id : %04X\n", info.VendorId, info.DeviceId);

            hr = _D3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

            if (SUCCEEDED(hr))
            {
                if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
                {
                    vertex_processing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
                }
                else
                {
                    // check vendor and device ID and enable SW vertex processing 
                    // for Intel(R) Extreme Graphics cards

                    if (SUCCEEDED(hr)) // if GetAdapterIdentifier SUCCEEDED
                    {
                        if (IsValidIntelCard(info.VendorId, info.DeviceId))
                        {
                            vertex_processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
                        }
                        else
                        {
                            // this is something else
                            vertex_processing = E_MINSPEC;
                            Logger::Error("GPU does not meet minimum specs: Intel(R) 845G or Hardware T&L chip required\n");
                            ///                        return false;
                            return;
                        }
                    }
                }
            }
            else
            {
                Logger::Error("failed to get device caps:\n%s\n", D3D9ErrorText(hr));

                if (IsValidIntelCard(info.VendorId, info.DeviceId))
                    vertex_processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            }

            if (vertex_processing == E_FAIL)
            {
                Logger::Error("failed to identify GPU\n");
                ///            return false;
                return;
            }


            // detect debug DirectX runtime
            /*
            _debug_dx_runtime = false;

            HKEY    key_direct3d;

            if( ::RegOpenKeyA( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Direct3D", &key_direct3d ) == ERROR_SUCCESS )
            {
            DWORD   type;
            DWORD   data    = 0;
            DWORD   data_sz = sizeof(data);

            if( ::RegQueryValueExA( key_direct3d, "LoadDebugRuntime", UNUSED_PARAM, &type, (BYTE*)(&data), &data_sz ) == ERROR_SUCCESS )
            {
            _debug_dx_runtime = (data == 1) ? true : false;
            }
            }
            note( "using %s DirectX runtime\n", (_debug_dx_runtime) ? "debug" : "retail" );
            */

            // create device

            // CRAP: hardcoded params

            d3dPresentParams.Windowed = TRUE;
            d3dPresentParams.BackBufferFormat = D3DFMT_UNKNOWN;
            d3dPresentParams.BackBufferWidth = backbuf_width;
            d3dPresentParams.BackBufferHeight = backbuf_height;
            d3dPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
            d3dPresentParams.BackBufferCount = 1;
            d3dPresentParams.EnableAutoDepthStencil = TRUE;
            d3dPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
            d3dPresentParams.PresentationInterval = (use_vsync) ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

            // TODO: check z-buf formats and create most suitable

            D3DDEVTYPE  device = D3DDEVTYPE_HAL;
            UINT        adapter = D3DADAPTER_DEFAULT;

            // check if specified display-mode supported

            ///        _DetectVideoModes();
            /*
            if( !_PresentParam.Windowed )
            {
            bool    found = false;

            for( unsigned f=0; f<countof(_VideomodeFormat); ++f )
            {
            D3DFORMAT   fmt = _VideomodeFormat[f];

            for( unsigned m=0; m<_DisplayMode.count(); ++m )
            {
            if(     _DisplayMode[m].width == _PresentParam.BackBufferWidth
            &&  _DisplayMode[m].height == _PresentParam.BackBufferHeight
            &&  _DisplayMode[m].format == fmt
            )
            {
            found = true;
            break;
            }
            }

            if( found )
            {
            _PresentParam.BackBufferFormat = (D3DFORMAT)fmt;
            break;
            }
            }

            if( !found )
            {
            Log::Error( "rhi.DX9", "invalid/unsuported display mode %ux%u\n", _PresentParam.BackBufferWidth, _PresentParam.BackBufferHeight );
            ///                return false;
            return;
            }
            }
            */

            // create device

            if (SUCCEEDED(hr = _D3D9->CreateDevice(adapter,
                device,
                hWindow,
                vertex_processing,
                &d3dPresentParams,
                &d3d9Device
                )
                ))
            {
                if (SUCCEEDED(_D3D9->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &info)))
                {

                    Logger::Info("Adapter[%u]:\n  %s \"%s\"\n", adapter, info.DeviceName, info.Description);
                    Logger::Info("  Driver %u.%u.%u.%u\n",
                        HIWORD(info.DriverVersion.HighPart),
                        LOWORD(info.DriverVersion.HighPart),
                        HIWORD(info.DriverVersion.LowPart),
                        LOWORD(info.DriverVersion.LowPart)
                        );
                }

            }
            else
            {
                Logger::Error("failed to create device:\n%s\n", D3D9ErrorText(hr));
            }

            rendererParams.context = d3d9Device;
            rendererParams.endFrameFunc = &EndFrameDX9;
        }
        else
        {
            Logger::Error("failed to create Direct3D object\n");
        }
    }

    void EndFrameGL()
    {
        DVASSERT(deviceContext);
        SwapBuffers(deviceContext);
    }

    void MakeCurrentGL()
    {
        DVASSERT(deviceContext);
        DVASSERT(glRenderContext);
        wglMakeCurrent(deviceContext, glRenderContext);
    }

    void CoreWin32Platform::CreateGLContext()
    {
        DVASSERT(deviceContext);

        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd
            1,                                // version number
            PFD_DRAW_TO_WINDOW |              // support window
            PFD_SUPPORT_OPENGL |              // support OpenGL
            PFD_DOUBLEBUFFER,                 // double buffered
            PFD_TYPE_RGBA,                    // RGBA type
            32,                               // 32-bit color depth
            0, 0, 0, 0, 0, 0,                 // color bits ignored

            0,                                // no alpha buffer
            0,                                // shift bit ignored
            0,                                // no accumulation buffer
            0, 0, 0, 0,                       // accum bits ignored
            24,                               // 24-bit z-buffer
            8,                                // 8-bit stencil buffer
            0,                                // no auxiliary buffer
            PFD_MAIN_PLANE,                   // main layer

            0,                                // reserved
            0, 0, 0                           // layer masks ignored
        };

        int  pixel_format = ChoosePixelFormat( deviceContext, &pfd );
        SetPixelFormat( deviceContext, pixel_format, &pfd );
        SetMapMode( deviceContext, MM_TEXT );


        glRenderContext = wglCreateContext( deviceContext );

        if (glRenderContext)
        {
            Logger::Info( "GL-context created\n" );

            /*
            GLint attr[] =
            {
            // here we ask for OpenGL 4.0
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            // forward compatibility mode
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            // uncomment this for Compatibility profile
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            // we are using Core profile here
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
            };
            */

            wglMakeCurrent(deviceContext, glRenderContext);

            rendererParams.makeCurrentFunc = &MakeCurrentGL;
            rendererParams.endFrameFunc = &EndFrameGL;

            glewExperimental = false;

            if (glewInit() == GLEW_OK)
            {
                /*
                HGLRC ctx4 = wglCreateContextAttribsARB( dc, 0, attr );
                if( ctx4  &&  wglMakeCurrent( dc, ctx4 ) )
                {
                //            wglDeleteContext( ctx );
                note( "using GL 4.0\n" );
                }
                */

                Logger::Info("GL inited\n");
                Logger::Info("  GL version   : %s", glGetString(GL_VERSION));
                Logger::Info("  GPU vendor   : %s", glGetString(GL_VENDOR));
                Logger::Info("  GPU          : %s", glGetString(GL_RENDERER));
                Logger::Info("  GLSL version : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
            }
            else
            {
                Logger::Error("GLEW init failed\n");
            }
        }
        else
        {
            Logger::Error( "can't create GL-context" );
        }
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
            
			Core::SystemProcessFrame();			

            uint32 elapsedTime = (uint32) (SystemTimer::Instance()->AbsoluteMS() - startTime);
            int32 sleepMs = 1;

            int32 fps = Renderer::GetDesiredFPS();
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

	int32 CoreWin32Platform::MoveTouchsToVector(USHORT buttsFlags, WPARAM wParam, LPARAM lParam, Vector<UIEvent> *outTouches)
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
		for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
		{
			if(it->tid == button)
			{
				isFind = true;

				it->physPoint.x = (float32)GET_X_LPARAM(lParam);
				it->physPoint.y = (float32)GET_Y_LPARAM(lParam);
				it->phase = phase;

				break;
			}
		}

		if(!isFind)
		{
			UIEvent newTouch;
			newTouch.tid = button;
			newTouch.physPoint.x = (float32)GET_X_LPARAM(lParam);
			newTouch.physPoint.y = (float32)GET_Y_LPARAM(lParam);
			newTouch.phase = phase;
			allTouches.push_back(newTouch);
		}

		for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
		{
			outTouches->push_back(*it);
		}

		if(phase == UIEvent::PHASE_ENDED || phase == UIEvent::PHASE_MOVE)
		{
			for(Vector<DAVA::UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); it++)
			{
				if(it->tid == button)
				{
					allTouches.erase(it);
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

	void CoreWin32Platform::OnMouseEvent(USHORT buttsFlags, WPARAM wParam, LPARAM lParam, USHORT buttonData)
    {
        Vector<DAVA::UIEvent> touches;
        int32 touchPhase = -1;

        if (HIWORD(wParam) || mouseButtonsDownMask > 0) // isPoint inside window or some clicks already captured
        {
            HandleMouseButtonsPressed(buttsFlags);
        }

        if(buttsFlags & RI_MOUSE_WHEEL)
        {
            UIEvent newTouch;
            newTouch.tid = 0;
            newTouch.physPoint.x = 0;
            newTouch.physPoint.y = ((SHORT)buttonData) / (float32)(WHEEL_DELTA);
            newTouch.phase = touchPhase = UIEvent::PHASE_WHEEL;
            touches.push_back(newTouch);
        }
        else
		{
            if(HIWORD(wParam) || mouseButtonsDownMask > 0) // HIWORD(wParam) - isPoint inside window
			{
			    touchPhase = MoveTouchsToVector(buttsFlags, wParam, lParam, &touches);
			}
		}

        if(touchPhase != -1)
            UIControlSystem::Instance()->OnInput(touchPhase, touches, allTouches);
#if RHI_COMPLETE
		if (RenderManager::Instance()->GetCursor() != 0 && mouseCursorShown)
		{
			ShowCursor(false);
			mouseCursorShown = false;
		}
		if (RenderManager::Instance()->GetCursor() == 0 && !mouseCursorShown)			
		{
			ShowCursor(false);
			mouseCursorShown = false;
		}
#endif // RHI_COMPLETE

		HandleMouseButtonsReleased(buttsFlags);
	}

	LRESULT CALLBACK CoreWin32Platform::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		CoreWin32Platform* core = static_cast< CoreWin32Platform* >(Core::Instance());
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA                     
#define WHEEL_DELTA 120
#endif

		switch (message) 
		{
		case WM_ERASEBKGND:
				return 0;

		case WM_KEYUP:
			{
				InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed((int32)wParam);
			};
			break;

		case WM_KEYDOWN:
			{
				BYTE allKeys[256];
				GetKeyboardState(allKeys);
	
				if ((allKeys[VK_MENU] & 0x80)
					&& (allKeys[VK_TAB] & 0x80))
				{
					ShowWindow(hWnd, SW_MINIMIZE);
				}

				Vector<DAVA::UIEvent> touches;

				DAVA::UIEvent ev;
				ev.keyChar = 0;
				ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
				ev.tapCount = 1;
				ev.tid = InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey((int32)wParam);

				touches.push_back(ev);

				UIControlSystem::Instance()->OnInput(0, touches, core->allTouches);
				touches.pop_back();
				UIControlSystem::Instance()->OnInput(0, touches, core->allTouches);

				InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed((int32)wParam);
			};
			break;

		case WM_CHAR:
		{
			if(wParam > 27) //TODO: remove this elegant check
			{
				Vector<DAVA::UIEvent> touches;

				DAVA::UIEvent ev;
				ev.keyChar = (char16)wParam;
				ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
				ev.tapCount = 1;
				ev.tid = 0;

				touches.push_back(ev);

				UIControlSystem::Instance()->OnInput(0, touches, core->allTouches);
				touches.pop_back();
				UIControlSystem::Instance()->OnInput(0, touches, core->allTouches);
			}
		}
		break;

        case WM_INPUT:
        {
            UINT dwSize;

            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, 
                sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == NULL)
                return 0;

            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, 
                sizeof(RAWINPUTHEADER)) != dwSize )
                OutputDebugString (TEXT("GetRawInputData does not return correct size !\n")); 

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if(raw->header.dwType == RIM_TYPEMOUSE && raw->data.mouse.usFlags == 0)
            {
                LONG x = raw->data.mouse.lLastX;
                LONG y = raw->data.mouse.lLastY;

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

                core->OnMouseEvent(raw->data.mouse.usButtonFlags, MAKEWPARAM(isMove, isInside), MAKELPARAM(x, y), raw->data.mouse.usButtonData); // only move, drag and wheel events
            }

            SafeDeleteArray(lpb);

            break;
        }
		case WM_MOUSEMOVE:
            //OnMouseEvent(message, wParam, lParam);
			break;

		case WM_NCMOUSEMOVE:
			if (!mouseCursorShown)
			{	
				ShowCursor(true);
				mouseCursorShown = true;
			}
			break;

		case WM_NCMOUSELEAVE:
			//ShowCursor(false);
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