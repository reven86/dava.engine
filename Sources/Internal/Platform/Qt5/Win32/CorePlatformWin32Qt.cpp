#include "CorePlatformWin32Qt.h"
#include "Utils/Utils.h"

#if defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{
void CoreWin32PlatformQt::InitArgs()
{
    SetCommandLine(GetCommandLineArgs());
}

void CoreWin32PlatformQt::Quit()
{
    PostQuitMessage(0);
}

int Core::Run(int argc, char* argv[], AppHandle handle)
{
    CoreWin32PlatformQt* core = new CoreWin32PlatformQt();
    core->CreateSingletons();
    core->InitArgs();

    return 0;
}

int Core::RunCmdTool(int argc, char* argv[], AppHandle handle)
{
    CoreWin32PlatformQt* core = new CoreWin32PlatformQt();

    core->EnableConsoleMode();
    core->CreateSingletons();

    core->InitArgs();

    Logger::Instance()->EnableConsoleMode();

    FrameworkDidLaunched();
    FrameworkWillTerminate();
    core->ReleaseSingletons();
    return 0;
}
}
#endif // #if defined(__DAVAENGINE_WIN32__)