#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Engine.h"

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"

#include "Logger/Logger.h"
#include "DAVAClassRegistrator.h"
#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Core/ApplicationCore.h"
#include "Core/PerformanceSettings.h"
#include "Platform/SystemTimer.h"
#include "UI/UIScreenManager.h"
#include "UI/UIControlSystem.h"
#include "Input/InputSystem.h"
#include "Debug/DVAssert.h"
#include "Render/2D/TextBlock.h"
#include "Debug/Replay.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Input/InputSystem.h"
#include "Platform/DPIHelper.h"
#include "Base/AllocatorFactory.h"
#include "Render/2D/FTFont.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Render/Image/ImageSystem.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "DLC/Downloader/DownloadManager.h"
#include "DLC/Downloader/CurlDownloader.h"
#include "Render/OcclusionQuery.h"
#include "Notification/LocalNotificationController.h"
#include "Platform/DeviceInfo.h"
#include "Render/Renderer.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"

namespace DAVA
{
namespace
{
Engine* engineSingleton = nullptr;
}

Engine* Engine::Instance()
{
    return engineSingleton;
}

Engine::Engine()
{
    //DVASSERT(engineSingleton == nullptr);
    engineSingleton = this;
    engineBackend = Private::EngineBackend::instance;
}

Engine::~Engine()
{
    engineBackend = nullptr;
    engineSingleton = nullptr;
}

Window* Engine::PrimaryWindow() const
{
    return engineBackend->primaryWindow;
}

void Engine::Init(bool consoleMode, const Vector<String>& modules)
{
    engineBackend->Init(consoleMode);

    // init modules

    new Logger();
    new AllocatorFactory();
    new JobManager();
    new FileSystem();
    FilePath::InitializeBundleName();
    FileSystem::Instance()->SetDefaultDocumentsDirectory();
    FileSystem::Instance()->CreateDirectory(FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);

    Logger::Info("SoundSystem init start");
    new SoundSystem();
    Logger::Info("SoundSystem init finish");

    if (consoleMode)
    {
        Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
    }
    DeviceInfo::InitializeScreenInfo();

    new LocalizationSystem();
    new SystemTimer();
    new Random();
    new AnimationManager();
    new FontManager();
    new UIControlSystem();
    new InputSystem();
    new PerformanceSettings();
    new VersionInfo();
    new ImageSystem();
    new FrameOcclusionQueryManager();
    new VirtualCoordinatesSystem();
    new RenderSystem2D();
    new UIScreenManager();

    Thread::InitMainThread();

    new DownloadManager();
    DownloadManager::Instance()->SetDownloader(new CurlDownloader());

    new LocalNotificationController();

    RegisterDAVAClasses();

    //new Net::NetCore();

    DAVA::VirtualCoordinatesSystem::Instance()->SetVirtualScreenSize(1024, 768);
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1024, 768, "Gfx");
}

int Engine::Run(IGame* gameObject)
{
    //DVASSERT(gameObject != nullptr);
    return engineBackend->Run(gameObject);
}

void Engine::Quit()
{
    engineBackend->Quit();
}

void Engine::RunAsyncOnMainThread(const Function<void()>& task)
{
    engineBackend->RunAsyncOnMainThread(task);
}

uint32 Engine::GetGlobalFrameIndex() const
{
    return 0;
}

const Vector<String>& Engine::GetCommandLine() const
{
    static Vector<String> x;
    return x;
}

bool Engine::IsConsoleMode() const
{
    return false;
}

void Engine::SetOptions(KeyedArchive* archiveOfOptions)
{
    options = archiveOfOptions;
}

KeyedArchive* Engine::GetOptions()
{
    return options;
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
