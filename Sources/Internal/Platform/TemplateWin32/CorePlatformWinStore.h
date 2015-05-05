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


#ifndef __DAVAENGINE_CORE_PLATFORM_WINSTORE_H__
#define __DAVAENGINE_CORE_PLATFORM_WINSTORE_H__

#include "Platform/PlatformDetection.h"
#if defined(__DAVAENGINE_WINDOWS_STORE__)

#include "DAVAEngine.h"
#include "agile.h"

namespace DAVA
{
//=============================================================================================
class CorePlatformWinStore : public Core, Noncopyable
{
public:
	CorePlatformWinStore();
	virtual ~CorePlatformWinStore();

	void Run();
	void InitArgs(); // if need

	virtual eScreenMode GetScreenMode() override;
	virtual void SwitchScreenToMode(eScreenMode screenMode) override;

	virtual void GetAvailableDisplayModes(List<DisplayMode> & availableModes) override;
	virtual void ToggleFullscreen();
	virtual DisplayMode FindBestMode(const DisplayMode & requestedMode) override;
	virtual DisplayMode GetCurrentDisplayMode() override;
	virtual void Quit() override;
	virtual void SetIcon(int32 iconId) override;

	virtual eScreenOrientation GetScreenOrientation() override;
	virtual uint32 GetScreenDPI() override;

	virtual void GoBackground(bool isLock) override;
	virtual void GoForeground() override;
private:

};
//=============================================================================================
ref class WinStoreApplicationSource : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
//=============================================================================================
ref class WinStoreFrame sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	WinStoreFrame();

	// IFrameworkView Methods
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();

protected:
	// Application lifecycle event handlers.
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView,
		             Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender,
		              Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender,
		            Platform::Object^ args);
	// Window event handlers.
	void OnWindowActivationChanged(Windows::UI::Core::CoreWindow^ sender,
		                           Windows::UI::Core::WindowActivatedEventArgs^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender,
						Windows::UI::Core::CoreWindowEventArgs^ args
		);
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender,
							 Windows::UI::Core::WindowSizeChangedEventArgs^ args);

private:
	Platform::Agile<Windows::UI::Core::CoreWindow> win_store_frame;
	bool willQuit;
	bool isWindowClosed;
	bool isWindowVisible;
	GLsizei windowWidth;
	GLsizei windowHeight;

};


} // namespace DAVA

#endif // #if defined(__DAVAENGINE_WINDOWS_STORE__)
#endif // __DAVAENGINE_CORE_PLATFORM_WINSTORE_H__