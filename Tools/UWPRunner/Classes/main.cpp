#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN32__)

#include "Engine/Engine.h"

#include "UWPRunner.h"

using namespace DAVA;

int Process(Engine& e)
{
    bool succeed = false;
    PackageOptions options = ParseCommandLine();
    if (CheckOptions(options))
    {
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

    return succeed ? 1 : 0;
}

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, {});

    e.update.Connect([&e](float32)
                     {
                         int result = Process(e);
                         e.Quit(result);
                     });

    return e.Run();
}

#endif // defined(__DAVAENGINE_WIN32__)