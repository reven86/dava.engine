#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN32__)

#include "Core/Core.h"

#include "UWPRunner.h"

bool succeed = false;

int main(int argc, char* argv[])
{
    DAVA::Core::RunCmdTool(0, 0, 0);
    return succeed ? 0 : 1;
}

void FrameworkDidLaunched()
{
    PackageOptions options = ParseCommandLine();
    if (!CheckOptions(options))
    {
        return;
    }

    try
    {
        UWPRunner runner(options);
        runner.Run();
        succeed = runner.IsSucceed();
    }
    catch (std::exception& e)
    {
        DAVA::Logger::Error("%s", e.what());
    }
}

void FrameworkWillTerminate()
{
}

#endif // defined(__DAVAENGINE_WIN32__)