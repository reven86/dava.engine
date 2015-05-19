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

#include "Platform/PlatformDetection.h"
#if defined(__DAVAENGINE_WINDOWS_STORE__)

#include "CorePlatformWinStore.h"
#include <angle_windowsstore.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{

using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;

//------------------------------------------------------------------------------------------------------
//                                      Core
//------------------------------------------------------------------------------------------------------
int Core::Run(int argc, char * argv[], AppHandle handle)
{
	// create window
	auto winStoreApplicationSource = ref new WinStoreApplicationSource();
	CoreApplication::Run(winStoreApplicationSource);

	return 0;
}
//------------------------------------------------------------------------------------------------------
int Core::RunCmdTool(int argc, char * argv[], AppHandle handle)
{
	return 1;
}
//------------------------------------------------------------------------------------------------------
//                                      CorePlatformWinStore
//------------------------------------------------------------------------------------------------------
CorePlatformWinStore::CorePlatformWinStore()
{
}
//------------------------------------------------------------------------------------------------------
CorePlatformWinStore::~CorePlatformWinStore()
{
}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::InitArgs()
{
	SetCommandLine(UTF8Utils::EncodeToUTF8(::GetCommandLineW()));
}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::Run()
{

}
//------------------------------------------------------------------------------------------------------
Core::eScreenMode CorePlatformWinStore::GetScreenMode()
{
	return Core::MODE_FULLSCREEN;
}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::SwitchScreenToMode(eScreenMode screenMode)
{

}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::GetAvailableDisplayModes(List<DisplayMode> & availableModes)
{

}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::ToggleFullscreen()
{

}
//------------------------------------------------------------------------------------------------------
DisplayMode CorePlatformWinStore::FindBestMode(const DisplayMode & requestedMode)
{
	return DisplayMode(800, 600, 16, 0);
}
//------------------------------------------------------------------------------------------------------
DisplayMode CorePlatformWinStore::GetCurrentDisplayMode()
{
	return DisplayMode(800, 600, 16, 0);
}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::Quit()
{
    CoreApplication::Exit();
}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::SetIcon(int32 iconId)
{

}
//------------------------------------------------------------------------------------------------------
Core::eScreenOrientation CorePlatformWinStore::GetScreenOrientation()
{
	return Core::eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_RIGHT;
}
//------------------------------------------------------------------------------------------------------
uint32 CorePlatformWinStore::GetScreenDPI()
{
	return 1;
}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::GoBackground(bool isLock)
{

}
//------------------------------------------------------------------------------------------------------
void CorePlatformWinStore::GoForeground()
{

}
//------------------------------------------------------------------------------------------------------
//                          WinStoreApplicationSource
//------------------------------------------------------------------------------------------------------
IFrameworkView^ WinStoreApplicationSource::CreateView()
{
	return ref new WinStoreFrame();
}
//------------------------------------------------------------------------------------------------------
//                          WinStoreFrame
//------------------------------------------------------------------------------------------------------
WinStoreFrame::WinStoreFrame() : willQuit(true), isWindowClosed(true), isWindowVisible(false),
                                 windowWidth(800), windowHeight(600)
{
}
//------------------------------------------------------------------------------------------------------
// This method is called on application launch.
void WinStoreFrame::Initialize(CoreApplicationView^ applicationView)
{
    CorePlatformWinStore* core = new CorePlatformWinStore();
    core->InitArgs(); // if need
    core->CreateSingletons();

	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinStoreFrame::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &WinStoreFrame::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &WinStoreFrame::OnResuming);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	
	// Create resource
}
//------------------------------------------------------------------------------------------------------
// This method is called after Initialize.
void WinStoreFrame::SetWindow(CoreWindow^ window)
{
    uint32 w = static_cast<uint32>(window->Bounds.Width);
    uint32 h = static_cast<uint32>(window->Bounds.Height);

	win_store_frame = window;
	//init angle
	RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
	RenderManager::Instance()->Create(win_store_frame.Get());
    RenderManager::Instance()->Init(w, h);
    RenderSystem2D::Instance()->Init();
	FrameworkDidLaunched();
	//RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
	VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(w, h);
	VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(w, h);

	// Specify the cursor type as the standard arrow cursor.
	win_store_frame->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
	// Allow the application to respond when the window size changes.
	win_store_frame->Activated +=
		ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &WinStoreFrame::OnWindowActivationChanged);
	win_store_frame->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WinStoreFrame::OnWindowSizeChanged);
	win_store_frame->Closed +=
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &WinStoreFrame::OnWindowClosed);
}
//------------------------------------------------------------------------------------------------------
void WinStoreFrame::Load(Platform::String^ entryPoint)
{
}
//------------------------------------------------------------------------------------------------------
// This method is called after Load.
void WinStoreFrame::Run()
{
	Core::Instance()->SystemAppStarted();

	while (!isWindowClosed)
	{
		if (isWindowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			DAVA::uint64 startTime = DAVA::SystemTimer::Instance()->AbsoluteMS();
			RenderManager::Instance()->Lock();
			CorePlatformWinStore::Instance()->SystemProcessFrame();
			RenderManager::Instance()->Unlock();
			uint32 elapsedTime = (uint32)(SystemTimer::Instance()->AbsoluteMS() - startTime);
			int32 sleepMs = 1;

			int32 fps = RenderManager::Instance()->GetFPS();
			if (fps > 0)
			{
				sleepMs = (1000 / fps) - elapsedTime;
				if (sleepMs < 1)
				{
					sleepMs = 1;
				}
			}
			Sleep(sleepMs);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
	RenderManager::Instance()->Release();

	ApplicationCore * appCore = Core::Instance()->GetApplicationCore();
	if (appCore && appCore->OnQuit())
	{
		exit(0);
	}
	else
	{
		willQuit = true;
	}
	Core::Instance()->SystemAppFinished();
	
	FrameworkWillTerminate();
}
//------------------------------------------------------------------------------------------------------
// This method is called before the application exits.
void WinStoreFrame::Uninitialize()
{
}
//------------------------------------------------------------------------------------------------------
void WinStoreFrame::OnActivated(CoreApplicationView^ /* applicationView */, IActivatedEventArgs^ /* args */)
{
	// Activate the application window, making it visible and enabling it to receive events.
	CoreWindow::GetForCurrentThread()->Activate();
	isWindowClosed = FALSE;
	isWindowVisible = TRUE;
    Core::Instance()->SetIsActive(true);
}
//------------------------------------------------------------------------------------------------------
void WinStoreFrame::OnSuspending(Platform::Object^ /* sender */, SuspendingEventArgs^ args)
{
    Core::Instance()->SetIsActive(false);
    isWindowVisible = FALSE;
}
//------------------------------------------------------------------------------------------------------
void WinStoreFrame::OnResuming(Platform::Object^ /* sender */, Platform::Object^ /* args */)
{
    Core::Instance()->SetIsActive(true);
    isWindowVisible = TRUE;
}
//------------------------------------------------------------------------------------------------------
void WinStoreFrame::OnWindowActivationChanged(CoreWindow^ sender, WindowActivatedEventArgs^ args)
{
}
//------------------------------------------------------------------------------------------------------
void WinStoreFrame::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	isWindowClosed = TRUE;
}
//------------------------------------------------------------------------------------------------------
// This method is called whenever the application window size changes.
void WinStoreFrame::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{

}
//------------------------------------------------------------------------------------------------------

} // namespace DAVA

#endif // #if defined(__DAVAENGINE_WINDOWS_STORE__)