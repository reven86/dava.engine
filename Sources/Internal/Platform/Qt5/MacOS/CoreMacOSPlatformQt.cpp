#include "Base/BaseTypes.h"
#include "Platform/Qt5/MacOS/CoreMacOSPlatformQt.h"
#include "Platform/Qt5/QtLayer.h"

#if defined(__DAVAENGINE_MACOS__)

#include <ApplicationServices/ApplicationServices.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{
int Core::Run(int argc, char* argv[], AppHandle handle)
{
    DAVA::CoreMacOSPlatformQt* core = new DAVA::CoreMacOSPlatformQt();
    core->SetCommandLine(argc, argv);
    core->CreateSingletons();

    return 0;
}

int Core::RunCmdTool(int argc, char* argv[], AppHandle handle)
{
    DAVA::CoreMacOSPlatformQt* core = new DAVA::CoreMacOSPlatformQt();
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
