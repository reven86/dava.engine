#include "Base/Platform.h"
#include "Concurrency/Thread.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/LocalizationSystem.h"
#include "Job/JobManager.h"
#include "Logger/Logger.h"
#include "Network/NetCore.h"
#include "Platform/SystemTimer.h"
#include "Engine/Engine.h"

#include "ClientApplication.h"

using namespace DAVA;

int Process(Engine& e)
{
    EngineContext* context = e.GetContext();
    context->logger->SetLogLevel(DAVA::Logger::LEVEL_INFO);
    context->logger->EnableConsoleMode();

    ClientApplication cacheClient;
    bool parsed = cacheClient.ParseCommandLine(e.GetCommandLine());
    if (parsed)
    {
        cacheClient.Process();
    }

    return static_cast<int>(cacheClient.GetExitCode());
}

int GameMain(Vector<String> cmdLine)
{
    Vector<String> modules =
    {
      "JobManager",
      "NetCore",
      "LocalizationSystem"
    };
    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, modules);

    e.update.Connect([&e](float32)
                     {
                         int result = Process(e);
                         e.Quit(result);
                     });

    return e.Run();
}
