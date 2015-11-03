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

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Utils/Utils.h"

#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/DispatcherWinUAP.h"

using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::ViewManagement;

namespace DAVA
{

int Core::Run(int /*argc*/, char* /*argv*/[], AppHandle /*handle*/)
{
    std::unique_ptr<CorePlatformWinUAP> core = std::make_unique<CorePlatformWinUAP>();
    core->InitArgs();
    core->Run();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
void CorePlatformWinUAP::InitArgs()
{
    SetCommandLine(WStringToString(::GetCommandLineW()));
}

void CorePlatformWinUAP::Run()
{
    auto appStartCallback = ref new ApplicationInitializationCallback([this](ApplicationInitializationCallbackParams^) {
        xamlApp = ref new WinUAPXamlApp();
    });
    Application::Start(appStartCallback);
}

void CorePlatformWinUAP::Quit()
{
    xamlApp->SetQuitFlag();
}

Core::eScreenMode CorePlatformWinUAP::GetScreenMode()
{
    // will be called from UI thread
    ApplicationViewWindowingMode viewMode;
    auto func = [this, &viewMode] { viewMode = xamlApp->GetScreenMode(); };
    RunOnUIThreadBlocked(func);
    switch (viewMode)
    {
    case ApplicationViewWindowingMode::FullScreen:
        return eScreenMode::FULLSCREEN;
    case ApplicationViewWindowingMode::PreferredLaunchViewSize:
        return eScreenMode::WINDOWED;
    case ApplicationViewWindowingMode::Auto:
    default:
        // Unknown screen mode -> return default value
        return eScreenMode::FULLSCREEN;
    }
}

bool CorePlatformWinUAP::SetScreenMode(eScreenMode screenMode)
{
    switch (screenMode)
    {
    case DAVA::Core::eScreenMode::FULLSCREEN:
        RunOnUIThread([this]() { xamlApp->SetScreenMode(ApplicationViewWindowingMode::FullScreen); });
        return true;
    case DAVA::Core::eScreenMode::WINDOWED_FULLSCREEN:
        Logger::Error("Unimplemented screen mode");
        return false;
    case DAVA::Core::eScreenMode::WINDOWED:
        RunOnUIThread([this]() { xamlApp->SetScreenMode(ApplicationViewWindowingMode::PreferredLaunchViewSize); });
        return true;
    default:
        DVASSERT_MSG(false, "Unknown screen mode");
        return false;
    }
}

DisplayMode CorePlatformWinUAP::GetCurrentDisplayMode()
{
    Windows::Foundation::Size screenSize;
    auto func = [this, &screenSize] { screenSize = xamlApp->GetCurrentScreenSize(); };
    RunOnUIThreadBlocked(func);
    return DisplayMode(static_cast<int32>(screenSize.Width), static_cast<int32>(screenSize.Height), DisplayMode::DEFAULT_BITS_PER_PIXEL, DisplayMode::DEFAULT_DISPLAYFREQUENCY);
}

void CorePlatformWinUAP::SetScreenScaleMultiplier(float32 multiplier)
{
    Core::SetScreenScaleMultiplier(multiplier);
    xamlApp->ResetScreen();
}

bool CorePlatformWinUAP::GetCursorVisibility()
{
    return xamlApp->GetCursorVisible();
}

InputSystem::eMouseCaptureMode CorePlatformWinUAP::GetMouseCaptureMode()
{
    return xamlApp->GetMouseCaptureMode();
}

bool CorePlatformWinUAP::SetMouseCaptureMode(InputSystem::eMouseCaptureMode mode)
{
    RunOnUIThreadBlocked([this, mode]() {
        if (xamlApp->SetMouseCaptureMode(mode))
        {
            xamlApp->SetCursorVisible(mode != InputSystem::eMouseCaptureMode::PINING);
        }
    });
    return GetMouseCaptureMode() == mode;
}

void CorePlatformWinUAP::SetDelegate(PushNotificationDelegate* dlg)
{
    xamlApp->SetDelegate(dlg);
}

bool CorePlatformWinUAP::IsUIThread() const
{
    return xamlApp->UIThreadDispatcher()->HasThreadAccess;
}

void CorePlatformWinUAP::RunOnUIThread(std::function<void()>&& fn, bool blocked)
{
    if (!blocked)
    {
        xamlApp->UIThreadDispatcher()->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(std::forward<std::function<void()>>(fn)));
    }
    else
    {
        DispatcherWinUAP::BlockingTaskWrapper wrapper = xamlApp->MainThreadDispatcher()->GetBlockingTaskWrapper(std::forward<std::function<void()>>(fn));
        RunOnUIThread([&wrapper]() { wrapper.RunTask(); });
        wrapper.WaitTaskComplete();
    }
}

void CorePlatformWinUAP::RunOnMainThread(std::function<void()>&& fn, bool blocked)
{
    if (!blocked)
    {
        xamlApp->MainThreadDispatcher()->RunAsync(std::forward<std::function<void()>>(fn));
    }
    else
    {
        xamlApp->MainThreadDispatcher()->RunAsyncAndWait(std::forward<std::function<void()>>(fn));
    }
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
