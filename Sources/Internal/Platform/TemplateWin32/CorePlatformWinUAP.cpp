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

#include "Platform/TemplateWin32/WinUAPXamlApp.h"
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
    ApplicationViewWindowingMode viewMode = xamlApp->GetScreenMode();
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
    switch (screenMode)
    {
    case Core::MODE_FULLSCREEN:
        xamlApp->SetScreenMode(ApplicationViewWindowingMode::FullScreen);
        break;
    case Core::MODE_WINDOWED:
        xamlApp->SetScreenMode(ApplicationViewWindowingMode::PreferredLaunchViewSize);
        break;
    default:
        xamlApp->SetScreenMode(ApplicationViewWindowingMode::FullScreen);
        break;
    }
}

Core::eScreenOrientation CorePlatformWinUAP::GetScreenOrientation()
{
    switch (xamlApp->GetDisplayOrientation())
    {
    case Windows::Graphics::Display::DisplayOrientations::None:
        return eScreenOrientation::SCREEN_ORIENTATION_TEXTURE;
    case Windows::Graphics::Display::DisplayOrientations::Landscape:
        return eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_RIGHT;
    case Windows::Graphics::Display::DisplayOrientations::Portrait:
        return eScreenOrientation::SCREEN_ORIENTATION_PORTRAIT;
    case Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped:
        return eScreenOrientation::SCREEN_ORIENTATION_LANDSCAPE_LEFT;
    case Windows::Graphics::Display::DisplayOrientations::PortraitFlipped:
        return eScreenOrientation::SCREEN_ORIENTATION_TEXTURE;
    default:
        return eScreenOrientation::SCREEN_ORIENTATION_TEXTURE;
    }
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
