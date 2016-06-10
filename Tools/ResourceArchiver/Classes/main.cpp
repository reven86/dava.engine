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
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);
    DAVA::Logger::Instance()->EnableConsoleMode();

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
