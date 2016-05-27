#include "CEFDavaRenderApp.h"
#include "Core/Core.h"

cef_main_args_t mainArgs;
int returnValue;
int RunCEFProcessHelper();

#ifdef __DAVAENGINE_WINDOWS__

int main(int argc, char* argv[])
{
    mainArgs.instance = GetModuleHandle(nullptr);
    CefMainArgs args(mainArgs);
    returnValue = CefExecuteProcess(args, new CEFDavaRenderApp, nullptr);
    return returnValue;
}

#else

int main(int argc, char* argv[])
{
    mainArgs.argc = argc;
    mainArgs.argv = argv;
    return RunCEFProcessHelper();
}

#endif

int RunCEFProcessHelper()
{
    DAVA::Core::RunCmdTool(0, 0, 0);
    return returnValue;
}

void FrameworkDidLaunched()
{
    CefMainArgs args(mainArgs);
    returnValue = CefExecuteProcess(args, new CEFDavaRenderApp, nullptr);
}

void FrameworkWillTerminate()
{
}
