#include <Base/BaseTypes.h>
#include <Engine/Engine.h>
#include <Logger/Logger.h>
#include <Debug/DVAssertDefaultHandlers.h>

#include "CommandLineApplication.h"
#include "ArchivePackTool.h"
#include "ArchiveUnpackTool.h"
#include "ArchiveListTool.h"
#include "ResultCodes.h"

int DAVAMain(DAVA::Vector<DAVA::String> cmdline)
{
#if !defined(__DAVAENGINE_MACOS__) && !defined(__DAVAENGINE_WIN32__)
#error "ResourceArchiver compiles only for win32 or macos"
#endif

    using namespace DAVA;

    Assert::SetupDefaultHandlers();

    Vector<String> modules = {
        "JobManager",
        "NetCore",
        "LocalizationSystem"
    };

    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, modules, nullptr);

    const EngineContext* context = e.GetContext();
    context->logger->SetLogLevel(Logger::LEVEL_INFO);
    context->logger->EnableConsoleMode();

    e.update.Connect([&e](DAVA::float32)
                     {
                         CommandLineApplication app("ResourceArchiver");
                         app.SetParseErrorCode(ResourceArchiverResult::ERROR_WRONG_COMMAND_LINE);
                         app.AddTool(std::unique_ptr<CommandLineTool>(new ArchivePackTool));
                         app.AddTool(std::unique_ptr<CommandLineTool>(new ArchiveUnpackTool));
                         app.AddTool(std::unique_ptr<CommandLineTool>(new ArchiveListTool));
                         int retCode = app.Process(e.GetCommandLine());
                         e.QuitAsync(retCode);
                     });

    return e.Run();
}
