#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Engine.h"

#include "Engine/EngineContext.h"
#include "Engine/Window.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/PlatformCore.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Render/Renderer.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

#include "Logger/Logger.h"
#include "DAVAClassRegistrator.h"
#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Core/ApplicationCore.h"
#include "Core/Core.h"
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
#include "Notification/LocalNotificationController.h"
#include "Platform/DeviceInfo.h"
#include "Render/Renderer.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"
#include "Network/NetCore.h"
#include "PackManager/Private/PackManagerImpl.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "Engine/Private/Android/AndroidBridge.h"
#endif

#include "UI/UIEvent.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
namespace Private
{
EngineBackend* EngineBackend::instance = nullptr;

EngineBackend* EngineBackend::Instance()
{
    return instance;
}

EngineBackend::EngineBackend(const Vector<String>& cmdargs)
    : dispatcher(new MainDispatcher(MakeFunction(this, &EngineBackend::EventHandler)))
    , platformCore(new PlatformCore(this))
    , context(new EngineContext)
    , cmdargs(cmdargs)
{
    DVASSERT(instance == nullptr);
    instance = this;

    context->logger = new Logger;
}

EngineBackend::~EngineBackend()
{
    instance = nullptr;
}

void EngineBackend::EngineCreated(Engine* e)
{
    engine = e;
    dispatcher->LinkToCurrentThread();
}

void EngineBackend::EngineDestroyed()
{
    engine = nullptr;
}

void EngineBackend::SetOptions(KeyedArchive* options_)
{
    options = options_;
}

KeyedArchive* EngineBackend::GetOptions()
{
    return options;
}

NativeService* EngineBackend::GetNativeService() const
{
    return platformCore->GetNativeService();
}

Vector<char*> EngineBackend::GetCommandLineAsArgv()
{
    Vector<char*> argv;
    argv.reserve(cmdargs.size());
    for (String& a : cmdargs)
    {
        argv.push_back(&*a.begin());
    }
    return argv;
}

Window* EngineBackend::InitializePrimaryWindow()
{
    DVASSERT(primaryWindow == nullptr);
    primaryWindow = new Window(this, true);
    justCreatedWindows.insert(primaryWindow);
    return primaryWindow;
}

void EngineBackend::Init(eEngineRunMode engineRunMode, const Vector<String>& modules)
{
    runMode = engineRunMode;

    // Do not initialize PlatformCore in console mode as console mode is fully
    // implemented in EngineBackend
    if (!IsConsoleMode())
    {
        platformCore->Init();
    }

    Thread::InitMainThread();
    // For now only next subsystems/modules are created on demand:
    //  - LocalizationSystem
    //  - JobManager
    //  - DownloadManager
    //  - NetCore
    //  - PackManager
    // Other subsystems are always created
    CreateSubsystems(modules);

    FilePath::InitializeBundleName();
    context->fileSystem->SetDefaultDocumentsDirectory();
    context->fileSystem->CreateDirectory(context->fileSystem->GetCurrentDocumentsDirectory(), true);

    if (!IsConsoleMode())
    {
        DeviceInfo::InitializeScreenInfo();
    }

    context->virtualCoordSystem->SetVirtualScreenSize(1024, 768);
    context->virtualCoordSystem->RegisterAvailableResourceSize(1024, 768, "Gfx");
    RegisterDAVAClasses();
}

int EngineBackend::Run()
{
    if (IsConsoleMode())
    {
        RunConsole();
    }
    else
    {
        platformCore->Run();
    }
    return exitCode;
}

void EngineBackend::Quit(int exitCode)
{
    this->exitCode = exitCode;
    switch (runMode)
    {
    case eEngineRunMode::GUI_STANDALONE:
        PostAppTerminate(false);
        break;
    case eEngineRunMode::GUI_EMBEDDED:
        Logger::Warning("Engine does not support Quit command in embedded mode");
        break;
    case eEngineRunMode::CONSOLE_MODE:
        quitConsole = true;
        break;
    default:
        break;
    }
}

void EngineBackend::SetCloseRequestHandler(const Function<bool(Window*)>& handler)
{
    closeRequestHandler = handler;
}

void EngineBackend::DispatchOnMainThread(const Function<void()>& task, bool blocking)
{
    MainDispatcherEvent e(MainDispatcherEvent::FUNCTOR);
    e.functor = task;
    blocking ? dispatcher->SendEvent(e) : dispatcher->PostEvent(e);
}

void EngineBackend::RunConsole()
{
    OnGameLoopStarted();
    while (!quitConsole)
    {
        OnFrameConsole();
        Thread::Sleep(1);
    }
    OnGameLoopStopped();
    OnEngineCleanup();
}

void EngineBackend::OnGameLoopStarted()
{
    engine->gameLoopStarted.Emit();
}

void EngineBackend::OnGameLoopStopped()
{
    DVASSERT(justCreatedWindows.empty());

    for (Window* w : dyingWindows)
    {
        delete w;
    }
    dyingWindows.clear();

    engine->gameLoopStopped.Emit();
    if (!IsConsoleMode())
    {
        if (Renderer::IsInitialized())
            Renderer::Uninitialize();
    }
}

void EngineBackend::OnEngineCleanup()
{
    engine->cleanup.Emit();

    DestroySubsystems();
    delete context;
    delete dispatcher;
    delete platformCore;
    context = nullptr;
    dispatcher = nullptr;
    platformCore = nullptr;
}

void EngineBackend::DoEvents()
{
    dispatcher->ProcessEvents();
    for (Window* w : aliveWindows)
    {
        w->FinishEventHandlingOnCurrentFrame();
    }
}

void EngineBackend::OnFrameConsole()
{
    context->systemTimer->Start();
    float32 frameDelta = context->systemTimer->FrameDelta();
    context->systemTimer->UpdateGlobalTime(frameDelta);

    DoEvents();
    engine->update.Emit(frameDelta);

    globalFrameIndex += 1;
}

int32 EngineBackend::OnFrame()
{
    context->systemTimer->Start();
    float32 frameDelta = context->systemTimer->FrameDelta();
    context->systemTimer->UpdateGlobalTime(frameDelta);

#if defined(__DAVAENGINE_QT__)
    if (Renderer::IsInitialized())
    {
        rhi::InvalidateCache();
    }
#endif

    DoEvents();
    if (!appIsSuspended)
    {
        if (Renderer::IsInitialized())
        {
            OnBeginFrame();
            OnUpdate(frameDelta);
            OnDraw();
            OnEndFrame();
        }
    }

    globalFrameIndex += 1;
    return Renderer::GetDesiredFPS();
}

void EngineBackend::OnBeginFrame()
{
    Renderer::BeginFrame();

    context->inputSystem->OnBeforeUpdate();
    engine->beginFrame.Emit();
}

void EngineBackend::OnUpdate(float32 frameDelta)
{
    context->localNotificationController->Update();
    context->animationManager->Update(frameDelta);

    for (Window* w : aliveWindows)
    {
        w->Update(frameDelta);
    }

    engine->update.Emit(frameDelta);
}

void EngineBackend::OnDraw()
{
    Renderer::GetRenderStats().Reset();
    context->renderSystem2D->BeginFrame();

    for (Window* w : aliveWindows)
    {
        w->Draw();
    }

    engine->draw.Emit();
    context->renderSystem2D->EndFrame();
}

void EngineBackend::OnEndFrame()
{
    context->inputSystem->OnAfterUpdate();
    engine->endFrame.Emit();
    Renderer::EndFrame();
}

void EngineBackend::OnWindowCreated(Window* window)
{
    {
        // Place window into alive window list
        size_t nerased = justCreatedWindows.erase(window);
        DVASSERT(nerased == 1);

        auto result = aliveWindows.insert(window);
        DVASSERT(result.second == true);
    }
    engine->windowCreated.Emit(window);
}

void EngineBackend::OnWindowDestroyed(Window* window)
{
    engine->windowDestroyed.Emit(window);

    // Remove window from alive window list and place it into dying window list to delete later
    size_t nerased = aliveWindows.erase(window);
    DVASSERT(nerased == 1);
    dyingWindows.insert(window);

    if (window->IsPrimary())
    {
        primaryWindow = nullptr;
    }

    if (aliveWindows.empty())
    { // No alive windows left, exit application
        platformCore->Quit();
    }
    else if (window->IsPrimary() && !IsEmbeddedGUIMode())
    { // Initiate app termination if primary window is destroyed, except embedded mode
        PostAppTerminate(false);
    }
}

void EngineBackend::EventHandler(const MainDispatcherEvent& e)
{
    switch (e.type)
    {
    case MainDispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case MainDispatcherEvent::APP_SUSPENDED:
        HandleAppSuspended(e);
        break;
    case MainDispatcherEvent::APP_RESUMED:
        HandleAppResumed(e);
        break;
    case MainDispatcherEvent::USER_CLOSE_REQUEST:
        HandleUserCloseRequest(e);
        break;
    case MainDispatcherEvent::APP_TERMINATE:
        HandleAppTerminate(e);
        break;
    default:
        if (e.window != nullptr)
        {
            e.window->EventHandler(e);
        }
        break;
    }
}

void EngineBackend::HandleAppTerminate(const MainDispatcherEvent& e)
{
    // Application can be terminated by several ways:
    //  1. application calls Engine::Quit
    //  2. application calls Window::Close for primary window
    //  3. user closes primary window (e.g. Alt+F4 key combination or mouse press on close button)
    //  4. system delivers unconditional termination request (e.g, android on activity finishing)
    //
    // EngineBackend receives termination request through MainDispatcherEvent::APP_TERMINATE event with
    // parameter triggeredBySystem which denotes termination request source: system (value 1) or user (value 0).
    // If termination request originates from user then EngineBackend calls PlatformCore to prepare for quit
    // (e.g. android implementation triggers activity finishing which in turn sends system termination request,
    // other platforms may simply repost termination request as if initiated by system).
    // If termination request originates from system then EngineBackend closes all active windows and waits
    // till all windows are closed. When last window is closed EngineBackend tells PlatformCore to quit which
    // usually means simply to exit game loop.
    // This sequence is invented for unification purpose.

    if (e.terminateEvent.triggeredBySystem != 0)
    {
        appIsTerminating = true;

        // Usually windows send blocking event about destruction and aliveWindows can be
        // modified while iterating over windows, so use such while construction.
        auto it = aliveWindows.begin();
        while (it != aliveWindows.end())
        {
            Window* w = *it;
            ++it;

            // Directly call Close for WindowBackend to tell important information that application is terminating
            w->GetBackend()->Close(true);
        }
    }
    else if (!appIsTerminating)
    {
        appIsTerminating = true;
        platformCore->PrepareToQuit();
    }
}

void EngineBackend::HandleAppSuspended(const MainDispatcherEvent& e)
{
    if (!appIsSuspended)
    {
        appIsSuspended = true;
        if (Renderer::IsInitialized())
            rhi::SuspendRendering();
        engine->suspended.Emit();
    }
}

void EngineBackend::HandleAppResumed(const MainDispatcherEvent& e)
{
    if (appIsSuspended)
    {
        appIsSuspended = false;
        if (Renderer::IsInitialized())
            rhi::ResumeRendering();
        engine->resumed.Emit();
    }
}

void EngineBackend::HandleUserCloseRequest(const MainDispatcherEvent& e)
{
    bool satisfyCloseRequest = true;
    if (closeRequestHandler != nullptr)
    {
        satisfyCloseRequest = closeRequestHandler(e.window);
    }

    if (satisfyCloseRequest)
    {
        if (e.window != nullptr)
        {
            e.window->Close();
        }
        else
        {
            Quit(0);
        }
    }
}

void EngineBackend::PostAppTerminate(bool triggeredBySystem)
{
    MainDispatcherEvent e(MainDispatcherEvent::APP_TERMINATE);
    e.timestamp = context->systemTimer->FrameStampTimeMS();
    e.terminateEvent.triggeredBySystem = triggeredBySystem;
    dispatcher->PostEvent(e);
}

void EngineBackend::PostUserCloseRequest()
{
    MainDispatcherEvent e(MainDispatcherEvent::USER_CLOSE_REQUEST);
    e.timestamp = context->systemTimer->FrameStampTimeMS();
    e.window = nullptr;
    dispatcher->PostEvent(e);
}

void EngineBackend::InitRenderer(Window* w)
{
    rhi::Api renderer = static_cast<rhi::Api>(options->GetInt32("renderer"));

    rhi::InitParam rendererParams;
    rendererParams.threadedRenderFrameCount = options->GetInt32("rhi_threaded_frame_count");
    if (rendererParams.threadedRenderFrameCount > 1)
    {
        rendererParams.threadedRenderEnabled = true;
    }

    rendererParams.maxIndexBufferCount = options->GetInt32("max_index_buffer_count");
    rendererParams.maxVertexBufferCount = options->GetInt32("max_vertex_buffer_count");
    rendererParams.maxConstBufferCount = options->GetInt32("max_const_buffer_count");
    rendererParams.maxTextureCount = options->GetInt32("max_texture_count");

    rendererParams.maxTextureSetCount = options->GetInt32("max_texture_set_count");
    rendererParams.maxSamplerStateCount = options->GetInt32("max_sampler_state_count");
    rendererParams.maxPipelineStateCount = options->GetInt32("max_pipeline_state_count");
    rendererParams.maxDepthStencilStateCount = options->GetInt32("max_depthstencil_state_count");
    rendererParams.maxRenderPassCount = options->GetInt32("max_render_pass_count");
    rendererParams.maxCommandBuffer = options->GetInt32("max_command_buffer_count");
    rendererParams.maxPacketListCount = options->GetInt32("max_packet_list_count");

    rendererParams.shaderConstRingBufferSize = options->GetInt32("shader_const_buffer_size");

    int32 physW = static_cast<int32>(w->GetRenderSurfaceWidth());
    int32 physH = static_cast<int32>(w->GetRenderSurfaceHeight());
    rendererParams.window = w->GetNativeHandle();
    rendererParams.width = physW;
    rendererParams.height = physH;
    rendererParams.scaleX = w->GetRenderSurfaceScaleX();
    rendererParams.scaleY = w->GetRenderSurfaceScaleY();

    w->InitCustomRenderParams(rendererParams);

    rhi::ShaderSourceCache::Load("~doc:/ShaderSource.bin");
    Renderer::Initialize(renderer, rendererParams);
    context->renderSystem2D->Init();
}

void EngineBackend::ResetRenderer(Window* w, bool resetToNull)
{
    rhi::ResetParam rendererParams;
    if (resetToNull == true)
    {
        rendererParams.window = nullptr;
        rendererParams.width = 0;
        rendererParams.height = 0;
        rendererParams.scaleX = 1.f;
        rendererParams.scaleY = 1.f;
    }
    else
    {
        int32 physW = static_cast<int32>(w->GetRenderSurfaceWidth());
        int32 physH = static_cast<int32>(w->GetRenderSurfaceHeight());
        rendererParams.window = w->GetNativeHandle();
        rendererParams.width = physW;
        rendererParams.height = physH;
        rendererParams.scaleX = w->GetRenderSurfaceScaleX();
        rendererParams.scaleY = w->GetRenderSurfaceScaleY();
    }
    Renderer::Reset(rendererParams);
}

void EngineBackend::DeinitRender(Window* w)
{
}

void EngineBackend::CreateSubsystems(const Vector<String>& modules)
{
    context->allocatorFactory = new AllocatorFactory();
    context->systemTimer = new SystemTimer();
    context->random = new Random();
    context->performanceSettings = new PerformanceSettings();
    context->versionInfo = new VersionInfo();
    context->fileSystem = new FileSystem();
    context->renderSystem2D = new RenderSystem2D();
    context->virtualCoordSystem = new VirtualCoordinatesSystem();
    context->uiControlSystem = new UIControlSystem();
    context->animationManager = new AnimationManager();

#if defined(__DAVAENGINE_ANDROID__)
    context->assetsManager = new AssetsManagerAndroid(AndroidBridge::GetApplicatiionPath());
#endif

    // Naive implementation of on demand module creation
    for (const String& m : modules)
    {
        if (m == "DownloadManager")
        {
            if (context->downloadManager == nullptr)
            {
                context->downloadManager = new DownloadManager(engine);
                context->downloadManager->SetDownloader(new CurlDownloader);
            }
        }
        else if (m == "JobManager")
        {
            if (context->jobManager == nullptr)
            {
                context->jobManager = new JobManager(engine);
            }
        }
        else if (m == "LocalizationSystem")
        {
            if (context->localizationSystem == nullptr)
            {
                context->localizationSystem = new LocalizationSystem;
            }
        }
        else if (m == "NetCore")
        {
            if (context->netCore == nullptr)
            {
                context->netCore = new Net::NetCore(engine);
            }
        }
        else if (m == "SoundSystem")
        {
            if (context->soundSystem == nullptr)
            {
                context->soundSystem = new SoundSystem(engine);
            }
        }
        else if (m == "PackManager")
        {
            if (context->packManager == nullptr)
            {
                context->packManager = new PackManagerImpl;
            }
        }
    }

    if (!IsConsoleMode())
    {
        context->fontManager = new FontManager();
        context->inputSystem = new InputSystem();
        context->uiScreenManager = new UIScreenManager();
        context->localNotificationController = new LocalNotificationController();
    }
}

void EngineBackend::DestroySubsystems()
{
    if (context->jobManager != nullptr)
    {
        // Wait job completion before releasing singletons
        // But client should stop its jobs on response to signals Engine::gameLoopStopped or Engine::cleanup
        context->jobManager->WaitWorkerJobs();
        context->jobManager->WaitMainJobs();
    }

    if (!IsConsoleMode())
    {
        context->localNotificationController->Release();
        context->uiScreenManager->Release();
        context->fontManager->Release();
        context->inputSystem->Release();
    }

    context->uiControlSystem->Release();
    context->animationManager->Release();
    context->virtualCoordSystem->Release();
    context->renderSystem2D->Release();
    context->performanceSettings->Release();
    context->random->Release();

    context->allocatorFactory->Release();
    context->versionInfo->Release();

    if (context->jobManager != nullptr)
        context->jobManager->Release();
    if (context->localizationSystem != nullptr)
        context->localizationSystem->Release();
    if (context->downloadManager != nullptr)
        context->downloadManager->Release();
    if (context->soundSystem != nullptr)
        context->soundSystem->Release();
    if (context->packManager != nullptr)
        delete context->packManager;

    // Finish network infrastructure
    // As I/O event loop runs in main thread so NetCore should run out loop to make graceful shutdown
    if (context->netCore != nullptr)
    {
        context->netCore->Finish(true);
        context->netCore->Release();
    }

#if defined(__DAVAENGINE_ANDROID__)
    context->assetsManager->Release();
#endif

    context->fileSystem->Release();
    context->systemTimer->Release();

    context->logger->Release();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
