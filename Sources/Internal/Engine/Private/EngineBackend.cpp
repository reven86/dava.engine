#include "Engine/Private/EngineBackend.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Engine/Window.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/PlatformCore.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

// Please place headers in alphabetic ascending order
#include "DAVAClassRegistrator.h"
#include "Analytics/Analytics.h"
#include "Analytics/LoggingBackend.h"
#include "ReflectionDeclaration/ReflectionDeclaration.h"
#include "Autotesting/AutotestingSystem.h"
#include "Base/AllocatorFactory.h"
#include "Base/ObjectFactory.h"
#include "Core/PerformanceSettings.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/DVAssert.h"
#include "Debug/Replay.h"
#include "Debug/Private/ImGui.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Debug/ProfilerCPU.h"
#include "DeviceManager/DeviceManager.h"
#include "DLC/Downloader/CurlDownloader.h"
#include "DLC/Downloader/DownloadManager.h"
#include "Engine/EngineSettings.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/KeyedArchive.h"
#include "Job/JobManager.h"
#include "Input/InputSystem.h"
#include "Logger/Logger.h"
#include "MemoryManager/MemoryManager.h"
#include "ModuleManager/ModuleManager.h"
#include "Network/NetCore.h"
#include "Notification/LocalNotificationController.h"
#include "DLCManager/Private/DLCManagerImpl.h"
#include "Platform/DeviceInfo.h"
#include "Platform/DPIHelper.h"
#include "Platform/Steam.h"
#include "PluginManager/PluginManager.h"
#include "Render/2D/FTFont.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Renderer.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Sound/SoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Time/SystemTimer.h"
#include "UI/UIEvent.h"
#include "UI/UIScreenManager.h"
#include "UI/UIControlSystem.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "Engine/Private/Android/AndroidBridge.h"
#endif

namespace DAVA
{
const EngineContext* GetEngineContext()
{
    return Private::EngineBackend::Instance()->GetContext();
}

Window* GetPrimaryWindow()
{
    return Private::EngineBackend::Instance()->GetPrimaryWindow();
}

void RunOnMainThreadAsync(const Function<void()>& task)
{
    Private::EngineBackend::Instance()->DispatchOnMainThread(task, false);
}

void RunOnMainThread(const Function<void()>& task)
{
    Private::EngineBackend* backend = Private::EngineBackend::Instance();

    DVASSERT(backend->IsRunning(), "RunOnMainThread should not be called outside of main loop (i.e. during `Engine::Run` execution)");
    backend->DispatchOnMainThread(task, true);
}

void RunOnUIThreadAsync(const Function<void()>& task)
{
    GetPrimaryWindow()->RunOnUIThreadAsync(task);
}

void RunOnUIThread(const Function<void()>& task)
{
    GetPrimaryWindow()->RunOnUIThread(task);
}

namespace Private
{
EngineBackend* EngineBackend::instance = nullptr;
bool EngineBackend::showingModalMessageBox = false;

EngineBackend* EngineBackend::Instance()
{
    return instance;
}

WindowBackend* EngineBackend::GetWindowBackend(Window* w)
{
    return w->windowBackend.get();
}

EngineBackend::EngineBackend(const Vector<String>& cmdargs)
    : dispatcher(new MainDispatcher(MakeFunction(this, &EngineBackend::EventHandler)))
    , platformCore(new PlatformCore(this))
    , context(new EngineContext)
    , cmdargs(cmdargs)
    , options(new KeyedArchive) // Ensure options never null
{
    DVASSERT(instance == nullptr);
    instance = this;

    // The following subsystems should be created earlier than other:
    //  - Logger, to log messages on startup
    //  - FileSystem, to load config files with init options
    //  - DeviceManager, to check what hatdware is available
    context->logger = new Logger;
    context->settings = new EngineSettings();
    context->fileSystem = new FileSystem;
    FilePath::InitializeBundleName();
    context->fileSystem->SetDefaultDocumentsDirectory();
    context->fileSystem->CreateDirectory(context->fileSystem->GetCurrentDocumentsDirectory(), true);

    // TODO: consider another way of DeviceManager initialization, as console apps possibly do not need DeviceManager
    context->deviceManager = new DeviceManager(this);
}

EngineBackend::~EngineBackend()
{
    instance = nullptr;
}

void EngineBackend::EngineCreated(Engine* engine_)
{
    engine = engine_;
    dispatcher->LinkToCurrentThread();
}

void EngineBackend::EngineDestroyed()
{
    engine = nullptr;
}

const KeyedArchive* EngineBackend::GetOptions() const
{
    return options.Get();
}

bool EngineBackend::IsSuspended() const
{
    return appIsSuspended;
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

void EngineBackend::Init(eEngineRunMode engineRunMode, const Vector<String>& modules, KeyedArchive* options_)
{
    DVASSERT(isInitialized == false && "Engine::Init is called more than once");

    runMode = engineRunMode;
    if (options_ != nullptr)
    {
        // For now simply transfer ownership without incrementing reference count
        options.Set(options_);
    }

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

    RegisterDAVAClasses();
    RegisterReflectionForBaseTypes();

    isInitialized = true;
}

int EngineBackend::Run()
{
    DVASSERT(isInitialized == true && "Engine::Init is not called");

    isRunning = true;

    if (IsConsoleMode())
    {
        RunConsole();
    }
    else
    {
        platformCore->Run();
    }

    isRunning = false;

    return exitCode;
}

void EngineBackend::Quit(int exitCode_)
{
    exitCode = exitCode_;
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
    }
    OnGameLoopStopped();
    OnEngineCleanup();
}

void EngineBackend::OnGameLoopStarted()
{
    Logger::Info("EngineBackend::OnGameLoopStarted: enter");

    engine->gameLoopStarted.Emit();

    Logger::Info("EngineBackend::OnGameLoopStarted: leave");
}

void EngineBackend::OnGameLoopStopped()
{
    Logger::Info("EngineBackend::OnGameLoopStopped: enter");

    DVASSERT(justCreatedWindows.empty());

    engine->gameLoopStopped.Emit();
    rhi::ShaderSourceCache::Save("~doc:/ShaderSource.bin");

    Logger::Info("EngineBackend::OnGameLoopStopped: leave");
}

void EngineBackend::OnEngineCleanup()
{
    Logger::Info("EngineBackend::OnEngineCleanup: enter");

    engine->cleanup.Emit();

    if (ImGui::IsInitialized())
        ImGui::Uninitialize();

    DestroySubsystems();

    for (Window* w : dyingWindows)
    {
        delete w;
    }
    dyingWindows.clear();
    primaryWindow = nullptr;

    if (Renderer::IsInitialized())
        Renderer::Uninitialize();

    delete context;
    delete dispatcher;
    delete platformCore;
    context = nullptr;
    dispatcher = nullptr;
    platformCore = nullptr;

    DAVA_MEMORY_PROFILER_FINISH();

    Logger::Info("EngineBackend::OnEngineCleanup: leave");
}

void EngineBackend::DoEvents()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_DO_EVENTS);
    dispatcher->ProcessEvents();
    for (Window* w : aliveWindows)
    {
        w->FinishEventHandlingOnCurrentFrame();
    }
}

void EngineBackend::OnFrameConsole()
{
    SystemTimer::StartFrame();
    float32 frameDelta = SystemTimer::GetFrameDelta();
    SystemTimer::ComputeRealFrameDelta();
    // TODO: UpdateGlobalTime is deprecated, remove later
    SystemTimer::UpdateGlobalTime(frameDelta);

    DoEvents();
    engine->update.Emit(frameDelta);

    // Notify memory profiler about new frame
    DAVA_MEMORY_PROFILER_UPDATE();

    globalFrameIndex += 1;
}

int32 EngineBackend::OnFrame()
{
    DAVA_PROFILER_CPU_SCOPE_WITH_FRAME_INDEX(ProfilerCPUMarkerName::ENGINE_ON_FRAME, globalFrameIndex);

    SystemTimer::StartFrame();
    float32 frameDelta = SystemTimer::GetFrameDelta();

    DoEvents();
    if (!appIsSuspended)
    {
        SystemTimer::ComputeRealFrameDelta();
        // TODO: UpdateGlobalTime is deprecated, remove later
        SystemTimer::UpdateGlobalTime(frameDelta);

        if (Renderer::IsInitialized())
        {
#if defined(__DAVAENGINE_QT__)
            rhi::InvalidateCache();
#endif
            Update(frameDelta);
            UpdateAndDrawWindows(frameDelta, false);
        }
    }
    else
    {
        // See comment to DrawSingleFrameWhileSuspended method
        if (drawSingleFrameWhileSuspended)
        {
            Logger::Info("EngineBackend::OnFrame, rendering single frame while suspended");
            ResumeRenderer();
            UpdateAndDrawWindows(frameDelta, true);
        }
        BackgroundUpdate(frameDelta);
    }

    drawSingleFrameWhileSuspended = false;

    // Notify memory profiler about new frame
    DAVA_MEMORY_PROFILER_UPDATE();

    globalFrameIndex += 1;
    return Renderer::GetDesiredFPS();
}

void EngineBackend::BackgroundUpdate(float32 frameDelta)
{
    engine->backgroundUpdate.Emit(frameDelta);
}

void EngineBackend::BeginFrame()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_BEGIN_FRAME);
    Renderer::BeginFrame();

    engine->beginFrame.Emit();
}

void EngineBackend::Update(float32 frameDelta)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_UPDATE);
    engine->update.Emit(frameDelta);

    context->localNotificationController->Update();
}

void EngineBackend::UpdateAndDrawWindows(float32 frameDelta, bool drawOnly)
{
    for (Window* w : aliveWindows)
    {
        BeginFrame();

        if (!drawOnly)
        {
            DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_UPDATE_WINDOW);
            w->Update(frameDelta);
        }

        {
            DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_DRAW_WINDOW);
            Renderer::GetRenderStats().Reset();
            w->Draw();
        }
        EndFrame();
    }
}

void EngineBackend::EndFrame()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ENGINE_END_FRAME);
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

        DVASSERT(std::find(aliveWindows.begin(), aliveWindows.end(), window) == aliveWindows.end());
        aliveWindows.push_back(window);
    }
    engine->windowCreated.Emit(window);

    window->visibilityChanged.Connect(this, &EngineBackend::OnWindowVisibilityChanged);
}

void EngineBackend::OnWindowDestroyed(Window* window)
{
    Logger::Info("EngineBackend::OnWindowDestroyed: enter");

    window->visibilityChanged.Disconnect(this);
    engine->windowDestroyed.Emit(window);

    // Remove window from alive window list
    auto it = std::find(aliveWindows.begin(), aliveWindows.end(), window);
    DVASSERT(it != aliveWindows.end());
    aliveWindows.erase(it);

    // Place it into dying window list to delete later
    dyingWindows.insert(window);

    if (aliveWindows.empty())
    { // No alive windows left, exit application
        platformCore->Quit();
    }
    else if (window->IsPrimary() && !IsEmbeddedGUIMode())
    { // Initiate app termination if primary window is destroyed, except embedded mode
        PostAppTerminate(false);
    }

    Logger::Info("EngineBackend::OnWindowDestroyed: leave");
}

void EngineBackend::OnWindowVisibilityChanged(Window* window, bool visible)
{
    // Update atLeastOneWindowIsVisible variable
    atLeastOneWindowIsVisible = false;
    for (Window* w : GetWindows())
    {
        if (w->IsVisible())
        {
            atLeastOneWindowIsVisible = true;
            break;
        }
    }

    // Update screen timeout
    if (atLeastOneWindowIsVisible)
    {
        // If at least one window is visible, switch to user setting
        platformCore->SetScreenTimeoutEnabled(screenTimeoutEnabled);
    }
    else
    {
        // Enable screen timeout if all windows are hidden
        platformCore->SetScreenTimeoutEnabled(true);
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
    case MainDispatcherEvent::BACK_NAVIGATION:
        HandleBackNavigation(e);
        break;
    case MainDispatcherEvent::USER_CLOSE_REQUEST:
        HandleUserCloseRequest(e);
        break;
    case MainDispatcherEvent::APP_TERMINATE:
        HandleAppTerminate(e);
        break;
    case MainDispatcherEvent::GAMEPAD_MOTION:
        context->inputSystem->HandleGamepadMotion(e);
        break;
    case MainDispatcherEvent::GAMEPAD_BUTTON_DOWN:
    case MainDispatcherEvent::GAMEPAD_BUTTON_UP:
        context->inputSystem->HandleGamepadButton(e);
        break;
    case MainDispatcherEvent::GAMEPAD_ADDED:
        context->inputSystem->HandleGamepadAdded(e);
        break;
    case MainDispatcherEvent::GAMEPAD_REMOVED:
        context->inputSystem->HandleGamepadRemoved(e);
        break;
    case MainDispatcherEvent::DISPLAY_CONFIG_CHANGED:
        context->deviceManager->HandleEvent(e);
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

    Logger::Info("EngineBackend::HandleAppTerminate: triggeredBySystem=%u", e.terminateEvent.triggeredBySystem);

    if (e.terminateEvent.triggeredBySystem != 0)
    {
        appIsTerminating = true;

        // WindowBackend::Close can lead to removing a window from aliveWindows list (inside of OnWindowDestroyed)
        // So copy the vector and iterate over the copy to avoid dealing with invalid iterators
        std::vector<Window*> aliveWindowsCopy = aliveWindows;
        for (Window* w : aliveWindowsCopy)
        {
            // Directly call Close for WindowBackend to tell important information that application is terminating
            GetWindowBackend(w)->Close(true);
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
        Logger::Info("EngineBackend::HandleAppSuspended: enter");

        appIsSuspended = true;
        SuspendRenderer();
        rhi::ShaderSourceCache::Save("~doc:/ShaderSource.bin");
        engine->suspended.Emit();

        Logger::Info("EngineBackend::HandleAppSuspended: leave");
    }
}

void EngineBackend::HandleAppResumed(const MainDispatcherEvent& e)
{
    if (appIsSuspended)
    {
        Logger::Info("EngineBackend::HandleAppResumed: enter");

        appIsSuspended = false;
        ResumeRenderer();
        engine->resumed.Emit();

        Logger::Info("EngineBackend::HandleAppResumed: leave");
    }
}

void EngineBackend::HandleBackNavigation(const MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.window = primaryWindow;
    uie.key = Key::BACK;
    uie.phase = UIEvent::Phase::KEY_DOWN;
    uie.device = eInputDevices::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    context->inputSystem->HandleInputEvent(&uie);
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
            e.window->CloseAsync();
        }
        else
        {
            Quit(0);
        }
    }
}

void EngineBackend::PostAppTerminate(bool triggeredBySystem)
{
    dispatcher->PostEvent(MainDispatcherEvent::CreateAppTerminateEvent(triggeredBySystem));
}

void EngineBackend::PostUserCloseRequest()
{
    dispatcher->PostEvent(MainDispatcherEvent::CreateUserCloseRequestEvent(nullptr));
}

void EngineBackend::InitRenderer(Window* w)
{
    rhi::Api renderer = static_cast<rhi::Api>(options->GetInt32("renderer", rhi::RHI_GLES2));
    DVASSERT(rhi::ApiIsSupported(renderer));

    if (!rhi::ApiIsSupported(renderer))
    {
        renderer = rhi::RHI_GLES2;
    }

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

    Size2f size = w->GetSize();
    Size2f surfSize = w->GetSurfaceSize();
    rendererParams.window = w->GetNativeHandle();
    rendererParams.width = static_cast<int32>(surfSize.dx);
    rendererParams.height = static_cast<int32>(surfSize.dy);
    rendererParams.scaleX = surfSize.dx / size.dx;
    rendererParams.scaleY = surfSize.dy / size.dy;

    rendererParams.renderingErrorCallbackContext = this;
    rendererParams.renderingErrorCallback = &EngineBackend::OnRenderingError;

    w->InitCustomRenderParams(rendererParams);

    rhi::ShaderSourceCache::Load("~doc:/ShaderSource.bin");
    Renderer::Initialize(renderer, rendererParams);
    context->renderSystem2D->Init();

    if (options->GetBool("init_imgui"))
        ImGui::Initialize();
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
        Size2f size = w->GetSize();
        Size2f surfSize = w->GetSurfaceSize();

        rendererParams.window = w->GetNativeHandle();
        rendererParams.width = static_cast<int32>(surfSize.dx);
        rendererParams.height = static_cast<int32>(surfSize.dy);
        rendererParams.scaleX = surfSize.dx / size.dx;
        rendererParams.scaleY = surfSize.dy / size.dy;
    }
    Renderer::Reset(rendererParams);
}

void EngineBackend::DeinitRender(Window* w)
{
}

void EngineBackend::UpdateDisplayConfig()
{
    context->deviceManager->UpdateDisplayConfig();
}

void EngineBackend::CreateSubsystems(const Vector<String>& modules)
{
    context->allocatorFactory = new AllocatorFactory();
    context->random = new Random();
    context->performanceSettings = new PerformanceSettings();
    context->versionInfo = new VersionInfo();
    context->renderSystem2D = new RenderSystem2D();
    context->uiControlSystem = new UIControlSystem();
    context->animationManager = new AnimationManager();
    context->fontManager = new FontManager();

#if defined(__DAVAENGINE_ANDROID__)
    context->assetsManager = new AssetsManagerAndroid(AndroidBridge::GetApplicationPath());
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
                context->soundSystem = CreateSoundSystem(engine);
            }
        }
        else if (m == "PackManager")
        {
            if (context->dlcManager == nullptr)
            {
                context->dlcManager = new DLCManagerImpl(engine);
            }
        }
    }

    if (!IsConsoleMode())
    {
        context->inputSystem = new InputSystem(engine);
        context->uiScreenManager = new UIScreenManager();
        context->localNotificationController = new LocalNotificationController();

#if defined(__DAVAENGINE_STEAM__)
        Steam::Init();
#endif
    }
    else
    {
        context->logger->EnableConsoleMode();
    }

    context->moduleManager = new ModuleManager(GetEngine());
    context->moduleManager->InitModules();

    context->pluginManager = new PluginManager(GetEngine());
    context->analyticsCore = new Analytics::Core;

#ifdef __DAVAENGINE_AUTOTESTING__
    context->autotestingSystem = new AutotestingSystem();
#endif
}

void EngineBackend::DestroySubsystems()
{
#ifdef __DAVAENGINE_AUTOTESTING__
    if (context->autotestingSystem != nullptr)
    {
        context->autotestingSystem->Release();
        context->autotestingSystem = nullptr;
    }
#endif

    if (!IsConsoleMode())
    {
#if defined(__DAVAENGINE_STEAM__)
        Steam::Deinit();
#endif
    }

    if (context->analyticsCore != nullptr)
    {
        delete context->analyticsCore;
        context->analyticsCore = nullptr;
    }
    if (context->settings != nullptr)
    {
        delete context->settings;
        context->settings = nullptr;
    }
    if (context->moduleManager != nullptr)
    {
        context->moduleManager->ShutdownModules();
        delete context->moduleManager;
        context->moduleManager = nullptr;
    }
    if (context->pluginManager != nullptr)
    {
        context->pluginManager->UnloadPlugins();
        delete context->pluginManager;
    }
    if (context->jobManager != nullptr)
    {
        // Wait job completion before releasing singletons
        // But client should stop its jobs on response to signals Engine::gameLoopStopped or Engine::cleanup
        context->jobManager->WaitWorkerJobs();
        context->jobManager->WaitMainJobs();
    }
    if (context->localNotificationController != nullptr)
    {
        context->localNotificationController->Release();
        context->localNotificationController = nullptr;
    }
    if (context->uiScreenManager != nullptr)
    {
        context->uiScreenManager->Release();
        context->uiScreenManager = nullptr;
    }
    if (context->uiControlSystem != nullptr)
    {
        context->uiControlSystem->Release();
        context->uiControlSystem = nullptr;
    }
    if (context->fontManager != nullptr)
    {
        context->fontManager->Release();
        context->fontManager = nullptr;
    }
    if (context->animationManager != nullptr)
    {
        context->animationManager->Release();
        context->animationManager = nullptr;
    }
    if (context->renderSystem2D != nullptr)
    {
        context->renderSystem2D->Release();
        context->renderSystem2D = nullptr;
    }
    if (context->performanceSettings != nullptr)
    {
        context->performanceSettings->Release();
        context->performanceSettings = nullptr;
    }
    if (context->random != nullptr)
    {
        context->random->Release();
        context->random = nullptr;
    }
    if (context->allocatorFactory != nullptr)
    {
        context->allocatorFactory->Release();
        context->allocatorFactory = nullptr;
    }
    if (context->versionInfo != nullptr)
    {
        context->versionInfo->Release();
        context->versionInfo = nullptr;
    }
    if (context->jobManager != nullptr)
    {
        context->jobManager->Release();
        context->jobManager = nullptr;
    }
    if (context->localizationSystem != nullptr)
    {
        context->localizationSystem->Release();
        context->localizationSystem = nullptr;
    }
    if (context->downloadManager != nullptr)
    {
        context->downloadManager->Release();
        context->downloadManager = nullptr;
    }
    if (context->soundSystem != nullptr)
    {
        context->soundSystem->Release();
        context->soundSystem = nullptr;
    }
    if (context->dlcManager != nullptr)
    {
        delete context->dlcManager;
        context->dlcManager = nullptr;
    }
    if (context->inputSystem != nullptr)
    {
        delete context->inputSystem;
        context->inputSystem = nullptr;
    }

    if (context->netCore != nullptr)
    {
        context->netCore->Release();
        context->netCore = nullptr;
    }

#if defined(__DAVAENGINE_ANDROID__)
    if (context->assetsManager != nullptr)
    {
        context->assetsManager->Release();
        context->assetsManager = nullptr;
    }
#endif

    if (context->fileSystem != nullptr)
    {
        context->fileSystem->Release();
        context->fileSystem = nullptr;
    }
    if (context->deviceManager != nullptr)
    {
        delete context->deviceManager;
        context->deviceManager = nullptr;
    }
    if (context->logger != nullptr)
    {
        context->logger->Release();
        context->logger = nullptr;
    }
}

void EngineBackend::OnRenderingError(rhi::RenderingError err, void* param)
{
    EngineBackend* self = static_cast<EngineBackend*>(param);
    self->engine->renderingError.Emit(err);

    // abort if signal was ignored
    String info = Format("Rendering is not possible and no handler found. Application will likely crash or hang now. Error: 0x%08x", static_cast<DAVA::uint32>(err));
    DVASSERT(0, info.c_str());
    Logger::Error("%s", info.c_str());
    abort();
}

void EngineBackend::AdjustSystemTimer(int64 adjustMicro)
{
    Logger::Info("System timer adjusted by %lld us", adjustMicro);
    SystemTimer::Adjust(adjustMicro);
}

void EngineBackend::SetScreenTimeoutEnabled(bool enabled)
{
    screenTimeoutEnabled = enabled;

    // Apply this setting only if at least one window is shown
    // Otherwise wait for it to be shown and apply it then
    if (atLeastOneWindowIsVisible)
    {
        platformCore->SetScreenTimeoutEnabled(screenTimeoutEnabled);
    }
}

bool EngineBackend::IsRunning() const
{
    return isRunning;
}

void EngineBackend::SuspendRenderer()
{
    if (Renderer::IsInitialized() && !rendererSuspended)
    {
        rhi::SuspendRendering();
        rendererSuspended = true;
    }
}

void EngineBackend::ResumeRenderer()
{
    if (Renderer::IsInitialized() && rendererSuspended)
    {
        rhi::ResumeRendering();
        rendererSuspended = false;
    }
}

void EngineBackend::DrawSingleFrameWhileSuspended()
{
    drawSingleFrameWhileSuspended = true;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
