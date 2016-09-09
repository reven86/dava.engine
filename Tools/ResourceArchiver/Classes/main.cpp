#if defined(__DAVAENGINE_COREV2__)

#include <Base/BaseTypes.h>
#include <Engine/EngineModule.h>

#include "CommandLineApplication.h"
#include "ArchivePackTool.h"
#include "ArchiveUnpackTool.h"
#include "ArchiveListTool.h"
#include "ResultCodes.h"

int GameMain(DAVA::Vector<DAVA::String> cmdline)
{
#if !defined(__DAVAENGINE_MACOS__) && !defined(__DAVAENGINE_WIN32__)
#error "ResourceArchiver compiles only for win32 or macos"
#endif

    using namespace DAVA;

    Vector<String> modules = {
        "JobManager",
        "NetCore",
        "LocalizationSystem"
    };

    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, modules);

    EngineContext* context = e.GetContext();
    context->logger->SetLogLevel(Logger::LEVEL_INFO);
    context->logger->EnableConsoleMode();

    CommandLineApplication app("ResourceArchiver");
    app.SetParseErrorCode(ResourceArchiverResult::ERROR_WRONG_COMMAND_LINE);
    app.AddTool(std::unique_ptr<CommandLineTool>(new ArchivePackTool));
    app.AddTool(std::unique_ptr<CommandLineTool>(new ArchiveUnpackTool));
    app.AddTool(std::unique_ptr<CommandLineTool>(new ArchiveListTool));

    return app.Process(cmdline);
}

#else

#include "Base/Platform.h"
#include "Concurrency/Thread.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/LocalizationSystem.h"
#include "Job/JobManager.h"
#include "Logger/Logger.h"
#include "Network/NetCore.h"
#include "Platform/SystemTimer.h"
#if defined(__DAVAENGINE_MACOS__)
#include "Platform/TemplateMacOS/CorePlatformMacOS.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Platform/TemplateWin32/CorePlatformWin32.h"
#endif //PLATFORMS
#include "CommandLineApplication.h"

#include "ArchivePackTool.h"
#include "ArchiveUnpackTool.h"
#include "ArchiveListTool.h"
#include "ResultCodes.h"

void CreateDAVA()
{
#if defined(__DAVAENGINE_MACOS__)
    new DAVA::CoreMacOSPlatform();
#elif defined(__DAVAENGINE_WIN32__)
    new DAVA::CoreWin32Platform();
#else // PLATFORMS
    static_assert(false, "Need create Core object");
#endif //PLATFORMS

    new DAVA::Logger();
    DAVA::Logger::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_WARNING);

    new DAVA::JobManager();
    new DAVA::FileSystem();
    DAVA::FilePath::InitializeBundleName();

    DAVA::FileSystem::Instance()->SetDefaultDocumentsDirectory();
    DAVA::FileSystem::Instance()->CreateDirectory(DAVA::FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);

    new DAVA::LocalizationSystem();
    new DAVA::SystemTimer();

    DAVA::Thread::InitMainThread();

    new DAVA::Net::NetCore();
}

void ReleaseDAVA()
{
    DAVA::JobManager::Instance()->WaitWorkerJobs();

    DAVA::Net::NetCore::Instance()->Finish(true);
    DAVA::Net::NetCore::Instance()->Release();

    DAVA::SystemTimer::Instance()->Release();
    DAVA::LocalizationSystem::Instance()->Release();
    DAVA::FileSystem::Instance()->Release();
    DAVA::JobManager::Instance()->Release();

    DAVA::Logger::Instance()->Release();

    DAVA::Core::Instance()->Release();
}

void FrameworkDidLaunched()
{
}

void FrameworkWillTerminate()
{
}

int main(int argc, char* argv[])
{
    CreateDAVA();

    CommandLineApplication app("ResourceArchiver");
    app.SetParseErrorCode(ResourceArchiverResult::ERROR_WRONG_COMMAND_LINE);
    app.AddTool(std::unique_ptr<CommandLineTool>(new ArchivePackTool));
    app.AddTool(std::unique_ptr<CommandLineTool>(new ArchiveUnpackTool));
    app.AddTool(std::unique_ptr<CommandLineTool>(new ArchiveListTool));

    int resultCode = app.Process(argc, argv);

    ReleaseDAVA();
    return resultCode;
}
#endif
