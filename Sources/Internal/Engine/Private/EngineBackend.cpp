#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Engine.h"

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

#include "UI/UIEvent.h"

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

int EngineBackend::Run()
{
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
    engine->signalGameLoopStarted.Emit();
}

void EngineBackend::OnGameLoopStopped()
{
    engine->signalGameLoopStopped.Emit();
}

void EngineBackend::DoEvents()
{
    dispatcher->ProcessEvents(MakeFunction(this, &EngineBackend::EventHandler));
}

void EngineBackend::OnFrame()
{
    DoEvents();

    OnBeginFrame();

    SystemTimer::Instance()->Start();
    float32 frameDelta = SystemTimer::Instance()->FrameDelta();
    SystemTimer::Instance()->UpdateGlobalTime(frameDelta);

    InputSystem::Instance()->OnBeforeUpdate();
    LocalNotificationController::Instance()->Update();
    DownloadManager::Instance()->Update();
    JobManager::Instance()->Update();
    SoundSystem::Instance()->Update(frameDelta);
    AnimationManager::Instance()->Update(frameDelta);
    UIControlSystem::Instance()->Update();
    engine->signalPreUpdate.Emit(frameDelta);

    engine->signalUpdate.Emit(frameDelta);

    InputSystem::Instance()->OnAfterUpdate();
    engine->signalPostUpdate.Emit(frameDelta);

    OnDraw();
    OnEndFrame();
}

void EngineBackend::OnBeginFrame()
{
    Renderer::BeginFrame();
    engine->signalBeginFrame.Emit();
}

void EngineBackend::OnDraw()
{
    RenderSystem2D::Instance()->BeginFrame();

    FrameOcclusionQueryManager::Instance()->ResetFrameStats();
    UIControlSystem::Instance()->Draw();
    FrameOcclusionQueryManager::Instance()->ProccesRenderedFrame();
    engine->signalDraw.Emit();

    RenderSystem2D::Instance()->EndFrame();
}

void EngineBackend::OnEndFrame()
{
    engine->signalEndFrame.Emit();
    Renderer::EndFrame();
}

Window* EngineBackend::CreateWindowFrontend(bool primary)
{
    //DVASSERT(!primary || primaryWindow == nullptr);
    Window* w = new Window(primary);
    return w;
}

void EngineBackend::EventHandler(const DispatcherEvent& e)
{
    switch (e.type)
    {
    case DispatcherEvent::MOUSE_MOVE:
        HandleMouseMove(e);
        break;
    case DispatcherEvent::MOUSE_BUTTON_DOWN:
    case DispatcherEvent::MOUSE_BUTTON_UP:
        HandleMouseClick(e);
        break;
    case DispatcherEvent::MOUSE_WHEEL:
        HandleMouseWheel(e);
        break;
    case DispatcherEvent::KEY_DOWN:
    case DispatcherEvent::KEY_UP:
        HandleKeyPress(e);
        break;
    case DispatcherEvent::KEY_CHAR:
        HandleKeyChar(e);
        break;
    case DispatcherEvent::FUNCTOR:
        e.functor();
        break;
    case DispatcherEvent::WINDOW_SIZE_SCALE_CHANGED:
        HandleWindowSizeChanged(e);
        break;
    case DispatcherEvent::WINDOW_FOCUS_CHANGED:
        HandleWindowFocusChanged(e);
        break;
    case DispatcherEvent::WINDOW_CREATED:
        HandleWindowCreated(e);
        break;
    case DispatcherEvent::WINDOW_DESTROYED:
        HandleWindowDestroyed(e);
        break;
    default:
        if (e.window != nullptr)
        {
            e.window->HandleEvent(e);
        }
        break;
    }
}

void EngineBackend::HandleWindowCreated(const DispatcherEvent& e)
{
    Logger::Debug("****** WINDOW_CREATED: w=%.1f, h=%.1f", e.sizeEvent.width, e.sizeEvent.height);
    e.window->PreHandleWindowCreated(e);
    CreateRenderer();
    e.window->HandleWindowCreated(e);
}

void EngineBackend::HandleWindowDestroyed(const DispatcherEvent& e)
{
    e.window->HandleWindowDestroyed(e);
    if (e.window->IsPrimary())
    {
        engine->Quit();
    }
}

void EngineBackend::HandleWindowSizeChanged(const DispatcherEvent& e)
{
    Logger::Debug("****** WINDOW_SIZE_SCALE_CHANGED: w=%.1f, h=%.1f", e.sizeEvent.width, e.sizeEvent.height);
    e.window->PreHandleSizeScaleChanged(e);
    ResetRenderer();
    e.window->HandleSizeScaleChanged(e);
}

void EngineBackend::HandleWindowFocusChanged(const DispatcherEvent& e)
{
    InputSystem::Instance()->GetKeyboard().ClearAllKeys();
    ClearMouseButtons();
    e.window->HandleEvent(e);
}

void EngineBackend::HandleMouseClick(const DispatcherEvent& e)
{
    bool pressed = e.type == DispatcherEvent::MOUSE_BUTTON_DOWN;

    Logger::Debug("****** %s: x=%.1f, y=%.1f, button=%d", pressed ? "MOUSE_BUTTON_DOWN" : "MOUSE_BUTTON_UP", e.mclickEvent.x, e.mclickEvent.y, e.mclickEvent.button);

    UIEvent uie;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.physPoint = Vector2(e.mclickEvent.x, e.mclickEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.mouseButton = static_cast<UIEvent::MouseButton>(e.mclickEvent.button);

    // NOTE: Taken from CoreWin32Platform::OnMouseClick

    bool isAnyButtonDownBefore = mouseButtonState.any();
    bool isButtonDown = uie.phase == UIEvent::Phase::BEGAN;
    uint32 buttonIndex = static_cast<uint32>(uie.mouseButton) - 1;
    mouseButtonState[buttonIndex] = isButtonDown;

    UIControlSystem::Instance()->OnInput(&uie);

    bool isAnyButtonDownAfter = mouseButtonState.any();

    //if (isAnyButtonDownBefore && !isAnyButtonDownAfter)
    //{
    //    ReleaseCapture();
    //}
    //else if (!isAnyButtonDownBefore && isAnyButtonDownAfter)
    //{
    //    SetCapture(hWindow);
    //}
}

void EngineBackend::HandleMouseWheel(const DispatcherEvent& e)
{
    Logger::Debug("****** MOUSE_WHEEL: x=%.1f, y=%.1f, delta=%d", e.mwheelEvent.x, e.mwheelEvent.y, e.mwheelEvent.delta);

    UIEvent uie;
    uie.phase = UIEvent::Phase::WHEEL;
    uie.physPoint = Vector2(e.mwheelEvent.x, e.mwheelEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.wheelDelta = { 0.0f, static_cast<float32>(e.mwheelEvent.delta) };

    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT))
    {
        std::swap(uie.wheelDelta.x, uie.wheelDelta.y);
    }

    UIControlSystem::Instance()->OnInput(&uie);
}

void EngineBackend::HandleMouseMove(const DispatcherEvent& e)
{
    //Logger::Debug("****** MOUSE_MOVE: x=%.1f, y=%.1f", e.mmoveEvent.x, e.mmoveEvent.y);

    UIEvent uie;
    uie.phase = UIEvent::Phase::MOVE;
    uie.physPoint = Vector2(e.mmoveEvent.x, e.mmoveEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.mouseButton = UIEvent::MouseButton::NONE;

    // NOTE: Taken from CoreWin32Platform::OnMouseMove
    if (mouseButtonState.any())
    {
        uie.phase = UIEvent::Phase::DRAG;

        uint32 firstButton = static_cast<uint32>(UIEvent::MouseButton::LEFT);
        uint32 lastButton = static_cast<uint32>(UIEvent::MouseButton::NUM_BUTTONS);
        for (uint32 buttonIndex = firstButton; buttonIndex <= lastButton; ++buttonIndex)
        {
            if (mouseButtonState[buttonIndex - 1])
            {
                uie.mouseButton = static_cast<UIEvent::MouseButton>(buttonIndex);
                UIControlSystem::Instance()->OnInput(&uie);
            }
        }
    }
    else
    {
        UIControlSystem::Instance()->OnInput(&uie);
    }
}

void EngineBackend::HandleKeyPress(const DispatcherEvent& e)
{
    bool pressed = e.type == DispatcherEvent::KEY_DOWN;

    Logger::Debug("****** %s", pressed ? "KEY_DOWN" : "KEY_UP");

    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

    UIEvent uie;
    uie.key = keyboard.GetDavaKeyForSystemKey(e.keyEvent.key);
    uie.device = UIEvent::Device::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    if (pressed)
        uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::KEY_DOWN_REPEAT : UIEvent::Phase::KEY_DOWN;
    else
        uie.phase = UIEvent::Phase::KEY_UP;

    UIControlSystem::Instance()->OnInput(&uie);
    keyboard.OnKeyPressed(uie.key);
}

void EngineBackend::HandleKeyChar(const DispatcherEvent& e)
{
    Logger::Debug("****** KEY_CHAR");

    UIEvent uie;
    uie.keyChar = static_cast<char32_t>(e.keyEvent.key);
    uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::CHAR_REPEAT : UIEvent::Phase::CHAR;
    uie.device = UIEvent::Device::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    UIControlSystem::Instance()->OnInput(&uie);
}

void EngineBackend::ClearMouseButtons()
{
    // NOTE: Taken from CoreWin32Platform::ClearMouseButtons

    UIEvent uie;
    uie.phase = UIEvent::Phase::ENDED;
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = SystemTimer::FrameStampTimeMS() / 1000.0;

    uint32 firstButton = static_cast<uint32>(UIEvent::MouseButton::LEFT);
    uint32 lastButton = static_cast<uint32>(UIEvent::MouseButton::NUM_BUTTONS);
    for (uint32 buttonIndex = firstButton; buttonIndex <= lastButton; ++buttonIndex)
    {
        if (mouseButtonState[buttonIndex - 1])
        {
            uie.mouseButton = static_cast<UIEvent::MouseButton>(buttonIndex);
            UIControlSystem::Instance()->OnInput(&uie);
        }
    }
    mouseButtonState.reset();
}

void EngineBackend::RunAsyncOnMainThread(const Function<void()>& task)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::FUNCTOR;
    e.window = nullptr;
    e.functor = task;
    dispatcher->PostEvent(e);
}

void EngineBackend::CreateRenderer()
{
    KeyedArchive* options = engine->options;

    rhi::InitParam rendererParams;
    rhi::Api renderer = rhi::RHI_DX9; //static_cast<rhi::Api>(options->GetInt32("renderer"));

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

    rendererParams.window = primaryWindow->NativeHandle();
    rendererParams.width = (int)primaryWindow->Width();
    rendererParams.height = (int)primaryWindow->Height();
    rendererParams.scaleX = primaryWindow->ScaleX();
    rendererParams.scaleY = primaryWindow->ScaleY();

    rhi::ShaderSourceCache::Load("~doc:/ShaderSource.bin");
    Renderer::Initialize(renderer, rendererParams);
    RenderSystem2D::Instance()->Init();

    VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
    virtSystem->SetInputScreenAreaSize(rendererParams.width, rendererParams.height);
    virtSystem->SetPhysicalScreenSize(rendererParams.width, rendererParams.height);
    virtSystem->EnableReloadResourceOnResize(true);
    virtSystem->ScreenSizeChanged();
}

void EngineBackend::ResetRenderer()
{
    rhi::ResetParam rendererParams;
    rendererParams.window = primaryWindow->NativeHandle();
    rendererParams.width = (int)primaryWindow->Width();
    rendererParams.height = (int)primaryWindow->Height();
    rendererParams.scaleX = primaryWindow->ScaleX();
    rendererParams.scaleY = primaryWindow->ScaleY();
    Renderer::Reset(rendererParams);

    VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
    virtSystem->SetInputScreenAreaSize(rendererParams.width, rendererParams.height);
    virtSystem->SetPhysicalScreenSize(rendererParams.width, rendererParams.height);
    virtSystem->UnregisterAllAvailableResourceSizes();
    virtSystem->RegisterAvailableResourceSize(rendererParams.width, rendererParams.height, "Gfx");
    virtSystem->ScreenSizeChanged();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
