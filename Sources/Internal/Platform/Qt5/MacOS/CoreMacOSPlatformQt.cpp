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


#include "Base/BaseTypes.h"
#include "Platform/Qt5/MacOS/CoreMacOSPlatformQt.h"
#include "Platform/Qt5/QtLayer.h"

#if defined(__DAVAENGINE_MACOS__)

#include <ApplicationServices/ApplicationServices.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();


namespace DAVA 
{

int Core::Run(int argc, char *argv[], AppHandle handle)
{
    DAVA::CoreMacOSPlatformQt * core = new DAVA::CoreMacOSPlatformQt();
    core->SetCommandLine(argc, argv);
    core->CreateSingletons();

    return 0;
}

int Core::RunCmdTool(int argc, char *argv[], AppHandle handle)
{
    DAVA::CoreMacOSPlatformQt * core = new DAVA::CoreMacOSPlatformQt();
    core->SetCommandLine(argc, argv);
    core->EnableConsoleMode();
    core->CreateSingletons();

    Logger::Instance()->EnableConsoleMode();

    FrameworkDidLaunched();
    FrameworkWillTerminate();

    core->ReleaseSingletons();
    return 0;
}

Core::eScreenMode CoreMacOSPlatformQt::GetScreenMode()
{
    return Core::eScreenMode::WINDOWED;
}

bool CoreMacOSPlatformQt::SetScreenMode(eScreenMode screenMode)
{
    Logger::FrameworkDebug("[CoreMacOSPlatformQt::SwitchScreenToMode()] has no sence for Qt");
    return screenMode == Core::eScreenMode::WINDOWED;
}

void CoreMacOSPlatformQt::Quit()
{
    QtLayer::Instance()->Quit();
}
	
}


#endif // #if defined(__DAVAENGINE_MACOS__)
