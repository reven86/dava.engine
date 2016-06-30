#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Engine.h"

#include "Engine/Public/EngineContext.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/PlatformCore.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"

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
#include "Render/OcclusionQuery.h"
#include "Notification/LocalNotificationController.h"
#include "Platform/DeviceInfo.h"
#include "Render/Renderer.h"
#include "UI/UIControlSystem.h"
#include "Job/JobManager.h"
#include "Network/NetCore.h"

#include "UI/UIEvent.h"

namespace DAVA
{
namespace Private
{
EngineBackend* EngineBackend::instance = nullptr;

EngineBackend* EngineBackend::Instance()
{
    return instance;
}

EngineBackend::EngineBackend(const Vector<String>& cmdargs_)
    : dispatcher(new Dispatcher)
    , platformCore(new PlatformCore(this))
    , context(new EngineContext)
    , cmdargs(cmdargs_)
{
    DVASSERT(instance == nullptr);
    instance = this;

    context->logger = new Logger;

#if defined(__DAVAENGINE_WIN_UAP__)
    CreatePrimaryWindowBackend();
#endif
}

EngineBackend::~EngineBackend()
{
    instance = nullptr;
}

void EngineBackend::EngineCreated(Engine* e)
{
    engine = e;
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

void EngineBackend::Init(bool consoleMode_, const Vector<String>& modules)
{
    consoleMode = consoleMode_;

    platformCore->Init();
    if (!consoleMode)
    {
#if !defined(__DAVAENGINE_WIN_UAP__)
        CreatePrimaryWindowBackend();
#endif
    }

    // For now only next subsystems/modules are created on demand:
    //  - LocalizationSystem
    //  - JobManager
    //  - DownloadManager
    //  - NetCore
    // Other subsystems are always created
    CreateSubsystems(modules);

    FilePath::InitializeBundleName();
    context->fileSystem->SetDefaultDocumentsDirectory();
    context->fileSystem->CreateDirectory(context->fileSystem->GetCurrentDocumentsDirectory(), true);

    if (!consoleMode)
    {
        DeviceInfo::InitializeScreenInfo();

        context->virtualCoordSystem->SetVirtualScreenSize(1024, 768);
        context->virtualCoordSystem->RegisterAvailableResourceSize(1024, 768, "Gfx");
    }

    Thread::InitMainThread();
    RegisterDAVAClasses();
}

int EngineBackend::Run()
{
    if (consoleMode)
    {
        RunConsole();
    }
    else
    {
        platformCore->Run();
    }
    return exitCode;
}

void EngineBackend::Quit(int exitCode_)
{
    exitCode = exitCode_;
    if (consoleMode)
    {
        quitConsole = true;
    }
    else
    {
        PostAppTerminate();
    }
}

void EngineBackend::RunAsyncOnMainThread(const Function<void()>& task)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::FUNCTOR;
    e.window = nullptr;
    e.functor = task;
    dispatcher->PostEvent(e);
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
    OnBeforeTerminate();
}

void EngineBackend::OnGameLoopStarted()
{
    engine->gameLoopStarted.Emit();
}

void EngineBackend::OnGameLoopStopped()
{
    engine->gameLoopStopped.Emit();
    if (!consoleMode)
    {
        Renderer::Uninitialize();
    }
}

void EngineBackend::OnBeforeTerminate()
{
    engine->beforeTerminate.Emit();

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
    dispatcher->ProcessEvents(MakeFunction(this, &EngineBackend::EventHandler));
    for (WindowBackend* w : windows)
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
}

int32 EngineBackend::OnFrame()
{
    context->systemTimer->Start();
    float32 frameDelta = context->systemTimer->FrameDelta();
    context->systemTimer->UpdateGlobalTime(frameDelta);

#if defined(__DAVAENGINE_QT__)
    rhi::InvalidateCache();
#endif

    DoEvents();
    OnBeginFrame();
    OnUpdate(frameDelta);
    OnDraw();
    OnEndFrame();

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

    for (WindowBackend* w : windows)
    {
        w->Update(frameDelta);
    }

    engine->update.Emit(frameDelta);
}

void EngineBackend::OnDraw()
{
    context->renderSystem2D->BeginFrame();

    context->frameOcclusionQueryManager->ResetFrameStats();
    for (WindowBackend* w : windows)
    {
        w->Draw();
    }
    context->frameOcclusionQueryManager->ProccesRenderedFrame();

    engine->draw.Emit();
    context->renderSystem2D->EndFrame();
}

void EngineBackend::OnEndFrame()
{
    context->inputSystem->OnAfterUpdate();
    engine->endFrame.Emit();
    Renderer::EndFrame();
}

void EngineBackend::EventHandler(const DispatcherEvent& e)
{
    switch (e.type)
    {
    case DispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case DispatcherEvent::WINDOW_CREATED:
        HandleWindowCreated(e);
        break;
    case DispatcherEvent::WINDOW_DESTROYED:
        HandleWindowDestroyed(e);
        break;
    case DispatcherEvent::APP_TERMINATE:
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

void EngineBackend::HandleWindowCreated(const DispatcherEvent& e)
{
    e.window->EventHandler(e);
    engine->windowCreated.Emit(e.window->GetWindow());
}

void EngineBackend::HandleWindowDestroyed(const DispatcherEvent& e)
{
    engine->windowDestroyed.Emit(e.window->GetWindow());
    e.window->EventHandler(e);

    size_t nerased = windows.erase(e.window);
    DVASSERT(nerased == 1);

    bool isPrimary = e.window->IsPrimary();
    delete e.window;

    if (isPrimary)
    {
        primaryWindow = nullptr;
        // If primary window is destroyed then terminate application
        PostAppTerminate();
    }

    if (windows.empty())
    {
        platformCore->Quit();
    }
}

void EngineBackend::HandleAppTerminate(const DispatcherEvent& e)
{
    for (WindowBackend* w : windows)
    {
        w->Close();
    }
}

void EngineBackend::PostAppTerminate()
{
    if (!appIsTerminating)
    {
        DispatcherEvent e;
        e.window = nullptr;
        e.type = DispatcherEvent::APP_TERMINATE;
        e.timestamp = context->systemTimer->FrameStampTimeMS();
        dispatcher->PostEvent(e);

        appIsTerminating = true;
    }
}

void EngineBackend::InitRenderer(WindowBackend* w)
{
    rhi::Api renderer = static_cast<rhi::Api>(options->GetInt32("renderer"));

    rhi::InitParam rendererParams;
    rendererParams.threadedRenderEnabled = true;
    rendererParams.threadedRenderFrameCount = 2;

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

    rhi::ShaderSourceCache::Load("~doc:/ShaderSource.bin");
    Renderer::Initialize(renderer, rendererParams);
    context->renderSystem2D->Init();
}

void EngineBackend::ResetRenderer(WindowBackend* w, bool resetToNull)
{
    rhi::ResetParam rendererParams;
    if (resetToNull)
    {
        rendererParams.window = nullptr;
        rendererParams.width = 0;
        rendererParams.height = 0;
        rendererParams.scaleX = 0.f;
        rendererParams.scaleY = 0.f;
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

void EngineBackend::DeinitRender(WindowBackend* w)
{
}

WindowBackend* EngineBackend::CreatePrimaryWindowBackend()
{
    DVASSERT(primaryWindow == nullptr);

    WindowBackend* window = new WindowBackend(this, true);
    windows.insert(window);

    primaryWindow = window;
    return window;
}

void EngineBackend::CreateSubsystems(const Vector<String>& modules)
{
    context->allocatorFactory = new AllocatorFactory();
    context->systemTimer = new SystemTimer();
    context->random = new Random();
    context->performanceSettings = new PerformanceSettings();
    context->versionInfo = new VersionInfo();
    context->fileSystem = new FileSystem();

    if (!consoleMode)
    {
        context->animationManager = new AnimationManager();
        context->fontManager = new FontManager();
        context->uiControlSystem = new UIControlSystem();
        context->inputSystem = new InputSystem();
        context->frameOcclusionQueryManager = new FrameOcclusionQueryManager();
        context->virtualCoordSystem = new VirtualCoordinatesSystem();
        context->renderSystem2D = new RenderSystem2D();
        context->uiScreenManager = new UIScreenManager();
        context->localNotificationController = new LocalNotificationController();
    }

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
    }
}

void EngineBackend::DestroySubsystems()
{
    if (context->jobManager != nullptr)
    {
        // Wait job completion before releasing singletons
        // But client should stop its jobs on response to signals Engine::gameLoopStopped or Engine::beforeTerminate
        context->jobManager->WaitWorkerJobs();
        context->jobManager->WaitMainJobs();
    }

    if (!consoleMode)
    {
        context->localNotificationController->Release();
        context->uiScreenManager->Release();
        context->uiControlSystem->Release();
        context->fontManager->Release();
        context->animationManager->Release();
        context->frameOcclusionQueryManager->Release();
        context->virtualCoordSystem->Release();
        context->renderSystem2D->Release();
        context->inputSystem->Release();
    }

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

    // Finish network infrastructure
    // As I/O event loop runs in main thread so NetCore should run out loop to make graceful shutdown
    if (context->netCore != nullptr)
    {
        context->netCore->Finish(true);
        context->netCore->Release();
    }

    context->fileSystem->Release();
    context->systemTimer->Release();

    context->logger->Release();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
