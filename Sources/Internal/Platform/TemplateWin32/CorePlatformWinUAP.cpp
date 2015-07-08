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
    if (IsUIThread())
    {
        func();
    }
    else
    {
        RunOnUIThreadBlocked(func);
    }
    switch (viewMode)
    {
    case ApplicationViewWindowingMode::FullScreen:
        return eScreenMode::MODE_FULLSCREEN;
    case ApplicationViewWindowingMode::PreferredLaunchViewSize:
        return eScreenMode::MODE_WINDOWED;
    case ApplicationViewWindowingMode::Auto:
        return eScreenMode::MODE_UNSUPPORTED;
    default:
        return eScreenMode::MODE_UNSUPPORTED;
    }
}

void CorePlatformWinUAP::SwitchScreenToMode(eScreenMode screenMode)
{
    ApplicationViewWindowingMode mode(ApplicationViewWindowingMode::PreferredLaunchViewSize);
    if (screenMode == Core::MODE_FULLSCREEN)
    {
        mode = ApplicationViewWindowingMode::FullScreen;
    }
    else if (screenMode == Core::MODE_WINDOWED)
    {
        mode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
    }
    if (IsUIThread())
    {
        xamlApp->SetScreenMode(mode);
    }
    else
    {
        RunOnUIThread([this, mode]() { xamlApp->SetScreenMode(mode); });
    }
}

DisplayMode CorePlatformWinUAP::GetCurrentDisplayMode()
{
    Windows::Foundation::Size screenSize;
    auto func = [this, &screenSize] { screenSize = xamlApp->GetCurrentScreenSize(); };
    if (IsUIThread())
    {
        func();
    }
    else
    {
        RunOnUIThreadBlocked(func);
    }
    return DisplayMode(static_cast<int32>(screenSize.Width), static_cast<int32>(screenSize.Height), DisplayMode::DEFAULT_BITS_PER_PIXEL, DisplayMode::DEFAULT_DISPLAYFREQUENCY);
}

void CorePlatformWinUAP::SetCursorPining(bool isPining)
{
    if (IsUIThread())
    {
        xamlApp->SetCursorPining(isPining);
        xamlApp->SetCursorVisible(!isPining);
    }
    else
    {
        RunOnUIThread([this, isPining]() {
        xamlApp->SetCursorPining(isPining);
        xamlApp->SetCursorVisible(!isPining);
        });
    }
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
