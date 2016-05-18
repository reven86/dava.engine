#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/IGame.h"

#include "Engine/Private/EngineBackend.h"
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

namespace DAVA
{
namespace Private
{
EngineBackend* EngineBackend::instance = nullptr;

EngineBackend::EngineBackend()
    : dispatcher(new Dispatcher)
    , platformCore(new PlatformCore)
{
    //DVASSERT(instance == nullptr);
    instance = this;
}

EngineBackend::~EngineBackend()
{
    delete dispatcher;
    delete platformCore;
    instance = nullptr;
}

void EngineBackend::Init(bool consoleMode_)
{
    consoleMode = consoleMode_;
    if (!consoleMode)
    {
#if defined(__DAVAENGINE_WIN32__)
        primaryWindow = CreateWindowFrontend(true);
#endif
    }
}

int EngineBackend::Run(IGame* gameObject)
{
    game = gameObject;
    if (!consoleMode)
    {
#if defined(__DAVAENGINE_WIN32__)
        platformCore->CreateNativeWindow(primaryWindow);
#endif
    }
    return platformCore->Run(consoleMode);
}

void EngineBackend::Quit()
{
    platformCore->Quit();
}

void EngineBackend::OnGameLoopStarted()
{
    game->OnGameLoopStarted();
}

void EngineBackend::OnGameLoopStopped()
{
    game->OnGameLoopStopped();
}

void EngineBackend::DoEvents()
{
    dispatcher->ProcessEvents(MakeFunction(this, &EngineBackend::EventHandler));
}

void EngineBackend::OnFrame()
{
    DoEvents();

    // ApplicationCore::BeginFrame
    Renderer::BeginFrame();

    SystemTimer::Instance()->Start();

    InputSystem::Instance()->OnBeforeUpdate();

    float32 frameDelta = SystemTimer::Instance()->FrameDelta();
    SystemTimer::Instance()->UpdateGlobalTime(frameDelta);

    LocalNotificationController::Instance()->Update();
    DownloadManager::Instance()->Update();
    JobManager::Instance()->Update();

    // ApplicationCore::Update
    SoundSystem::Instance()->Update(frameDelta);
    AnimationManager::Instance()->Update(frameDelta);
    UIControlSystem::Instance()->Update();

    // ApplicationCore::Draw
    RenderSystem2D::Instance()->BeginFrame();
    FrameOcclusionQueryManager::Instance()->ResetFrameStats();
    UIControlSystem::Instance()->Draw();
    FrameOcclusionQueryManager::Instance()->ProccesRenderedFrame();
    RenderSystem2D::Instance()->EndFrame();

    // ApplicationCore::EndFrame
    Renderer::EndFrame();

    game->OnUpdate(frameDelta);

    InputSystem::Instance()->OnAfterUpdate();
}

Window* EngineBackend::CreateWindowFrontend(bool primary)
{
    //DVASSERT(!primary || primaryWindow == nullptr);
    Window* w = new Window(primary);
    return w;
}

void EngineBackend::EventHandler(const DispatcherEvent& e)
{
    //if (e.window != nullptr)
    //{
    //    e.window->EventHandler(e);
    //}

    if (e.type == DispatcherEvent::WINDOW_FOCUS_CHANGED)
    {
        e.window->HandleEvent(e);
    }
    else if (e.type == DispatcherEvent::WINDOW_VISIBILITY_CHANGED)
    {
        e.window->HandleEvent(e);
    }
    else if (e.type == DispatcherEvent::WINDOW_SIZE_CHANGED)
    {
        if (e.window->sizeCome)
        {
            rhi::ResetParam params;
            params.width = (int)e.sizeEvent.width;
            params.height = (int)e.sizeEvent.height;
            params.window = e.window->NativeHandle();
            Renderer::Reset(params);

            VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
            virtSystem->SetInputScreenAreaSize(params.width, params.height);
            virtSystem->SetPhysicalScreenSize(params.width, params.height);
            virtSystem->ScreenSizeChanged();
        }
        else
        {
            rhi::InitParam rendererParams;
            rhi::Api renderer = rhi::RHI_DX11; //static_cast<rhi::Api>(options->GetInt32("renderer"));

            rendererParams.threadedRenderEnabled = true;
            rendererParams.threadedRenderFrameCount = 2;

            rendererParams.maxIndexBufferCount = 0; //options->GetInt32("max_index_buffer_count");
            rendererParams.maxVertexBufferCount = 0; //options->GetInt32("max_vertex_buffer_count");
            rendererParams.maxConstBufferCount = 0; //options->GetInt32("max_const_buffer_count");
            rendererParams.maxTextureCount = 0; //options->GetInt32("max_texture_count");

            rendererParams.maxTextureSetCount = 0; //options->GetInt32("max_texture_set_count");
            rendererParams.maxSamplerStateCount = 0; //options->GetInt32("max_sampler_state_count");
            rendererParams.maxPipelineStateCount = 0; //options->GetInt32("max_pipeline_state_count");
            rendererParams.maxDepthStencilStateCount = 0; //options->GetInt32("max_depthstencil_state_count");
            rendererParams.maxRenderPassCount = 0; //options->GetInt32("max_render_pass_count");
            rendererParams.maxCommandBuffer = 0; //options->GetInt32("max_command_buffer_count");
            rendererParams.maxPacketListCount = 0; //options->GetInt32("max_packet_list_count");

            rendererParams.shaderConstRingBufferSize = 0; //options->GetInt32("shader_const_buffer_size");

            rendererParams.window = e.window->NativeHandle();
            rendererParams.width = (int)e.sizeEvent.width; //primaryWindow->Width();
            rendererParams.height = (int)e.sizeEvent.height; //primaryWindow->Height();
            //rendererParams.scaleX = 1;
            //rendererParams.scaleY = 1;

            VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
            virtSystem->SetInputScreenAreaSize(rendererParams.width, rendererParams.height);
            virtSystem->SetPhysicalScreenSize(rendererParams.width, rendererParams.height);
            virtSystem->EnableReloadResourceOnResize(true);

            rhi::ShaderSourceCache::Load("~doc:/ShaderSource.bin");
            Renderer::Initialize(renderer, rendererParams);
            RenderSystem2D::Instance()->Init();
        }

        e.window->HandleEvent(e);
    }
    else if (e.type == DispatcherEvent::WINDOW_SCALE_CHANGED)
    {
        e.window->HandleEvent(e);
    }
    else if (e.type == DispatcherEvent::WINDOW_CLOSED)
    {
        e.window->HandleEvent(e);
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

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
