#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Window.h"

#include "Engine/EngineContext.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/WindowBackend.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"
#include "Input/InputSystem.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
Window::Window(Private::EngineBackend* engineBackend, bool primary)
    : engineBackend(engineBackend)
    , mainDispatcher(engineBackend->GetDispatcher())
    , windowBackend(new Private::WindowBackend(engineBackend, this))
    , isPrimary(primary)
{
}

Window::~Window() = default;

void Window::Resize(float32 w, float32 h)
{
    // Window cannot be resized in embedded mode as window lifetime
    // is controlled by highlevel framework
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->Resize(w, h);
    }
}

void Window::Close()
{
    // Window cannot be close in embedded mode as window lifetime
    // is controlled by highlevel framework
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->Close(false);
    }
}

void Window::SetTitle(const String& title)
{
    // It does not make sense to set window title in embedded mode
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->SetTitle(title);
    }
}

Engine* Window::GetEngine() const
{
    return engineBackend->GetEngine();
}

void* Window::GetNativeHandle() const
{
    return windowBackend->GetHandle();
}

WindowNativeService* Window::GetNativeService() const
{
    return windowBackend->GetNativeService();
}

void Window::RunAsyncOnUIThread(const Function<void()>& task)
{
    windowBackend->RunAsyncOnUIThread(task);
}

void Window::InitCustomRenderParams(rhi::InitParam& params)
{
    windowBackend->InitCustomRenderParams(params);
}

void Window::Update(float32 frameDelta)
{
    uiControlSystem->Update();
    update.Emit(this, frameDelta);
}

void Window::Draw()
{
    if (isVisible)
    {
        uiControlSystem->Draw();
    }
}

void Window::EventHandler(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;
    switch (e.type)
    {
    case MainDispatcherEvent::MOUSE_MOVE:
        HandleMouseMove(e);
        break;
    case MainDispatcherEvent::MOUSE_BUTTON_DOWN:
    case MainDispatcherEvent::MOUSE_BUTTON_UP:
        HandleMouseClick(e);
        break;
    case MainDispatcherEvent::MOUSE_WHEEL:
        HandleMouseWheel(e);
        break;
    case MainDispatcherEvent::TOUCH_DOWN:
    case MainDispatcherEvent::TOUCH_UP:
        HandleTouchClick(e);
        break;
    case MainDispatcherEvent::TOUCH_MOVE:
        HandleTouchMove(e);
        break;
    case MainDispatcherEvent::KEY_DOWN:
    case MainDispatcherEvent::KEY_UP:
        HandleKeyPress(e);
        break;
    case MainDispatcherEvent::KEY_CHAR:
        HandleKeyChar(e);
        break;
    case MainDispatcherEvent::WINDOW_SIZE_SCALE_CHANGED:
        HandleSizeChanged(e);
        break;
    case MainDispatcherEvent::WINDOW_FOCUS_CHANGED:
        HandleFocusChanged(e);
        break;
    case MainDispatcherEvent::WINDOW_VISIBILITY_CHANGED:
        HandleVisibilityChanged(e);
        break;
    case MainDispatcherEvent::WINDOW_CREATED:
        HandleWindowCreated(e);
        break;
    case MainDispatcherEvent::WINDOW_DESTROYED:
        HandleWindowDestroyed(e);
        break;
    default:
        break;
    }
}

void Window::FinishEventHandlingOnCurrentFrame()
{
    sizeEventHandled = false;
    windowBackend->TriggerPlatformEvents();
}

void Window::HandleWindowCreated(const Private::MainDispatcherEvent& e)
{
    CompressSizeChangedEvents(e);
    sizeEventHandled = true;

    Logger::FrameworkDebug("=========== WINDOW_CREATED: width=%.1f, height=%.1f, scaleX=%.3f, scaleY=%.3f", width, height, scaleX, scaleY);

    engineBackend->InitRenderer(this);

    EngineContext* context = engineBackend->GetEngineContext();
    inputSystem = context->inputSystem;
    uiControlSystem = context->uiControlSystem;
    virtualCoordSystem = context->virtualCoordSystem;
    virtualCoordSystem->EnableReloadResourceOnResize(true);

    UpdateVirtualCoordinatesSystem();

    engineBackend->OnWindowCreated(this);
    sizeScaleChanged.Emit(this, width, height, scaleX, scaleY);
}

void Window::HandleWindowDestroyed(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_DESTROYED");

    engineBackend->OnWindowDestroyed(this);

    inputSystem = nullptr;
    uiControlSystem = nullptr;
    virtualCoordSystem = nullptr;

    engineBackend->DeinitRender(this);
}

void Window::HandleSizeChanged(const Private::MainDispatcherEvent& e)
{
    if (!sizeEventHandled)
    {
        CompressSizeChangedEvents(e);
        sizeEventHandled = true;

        Logger::FrameworkDebug("=========== WINDOW_SIZE_SCALE_CHANGED: width=%.1f, height=%.1f, scaleX=%.3f, scaleY=%.3f", width, height, scaleX, scaleY);

        engineBackend->ResetRenderer(this, !windowBackend->IsWindowReadyForRender());
        if (windowBackend->IsWindowReadyForRender())
        {
            UpdateVirtualCoordinatesSystem();
            sizeScaleChanged.Emit(this, width, height, scaleX, scaleY);
        }
    }
}

void Window::CompressSizeChangedEvents(const Private::MainDispatcherEvent& e)
{
    // Look into dispatcher queue and compress size events into one event to allow:
    //  - single render init/reset call during one frame
    //  - emit signals about window creation or size changing immediately on event receiving
    using Private::MainDispatcherEvent;
    MainDispatcherEvent::WindowSizeEvent compressedSize(e.sizeEvent);
    mainDispatcher->ViewEventQueue([this, &compressedSize](const MainDispatcherEvent& e) {
        if (e.window == this && e.type == MainDispatcherEvent::WINDOW_SIZE_SCALE_CHANGED)
        {
            compressedSize.width = e.sizeEvent.width;
            compressedSize.height = e.sizeEvent.height;
            compressedSize.scaleX = e.sizeEvent.scaleX;
            compressedSize.scaleY = e.sizeEvent.scaleY;
        }
    });

    width = compressedSize.width;
    height = compressedSize.height;
    scaleX = compressedSize.scaleX;
    scaleY = compressedSize.scaleY;
}

void Window::UpdateVirtualCoordinatesSystem()
{
    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);
    int32 physW = static_cast<int32>(GetRenderSurfaceWidth());
    int32 physH = static_cast<int32>(GetRenderSurfaceHeight());

    virtualCoordSystem->SetInputScreenAreaSize(w, h);
    virtualCoordSystem->SetPhysicalScreenSize(physW, physH);
    virtualCoordSystem->SetVirtualScreenSize(w, h);
    virtualCoordSystem->UnregisterAllAvailableResourceSizes();
    virtualCoordSystem->RegisterAvailableResourceSize(w, h, "Gfx");
    virtualCoordSystem->ScreenSizeChanged();
}

void Window::HandleFocusChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_FOCUS_CHANGED: state=%s", e.stateEvent.state ? "got_focus" : "lost_focus");

    inputSystem->GetKeyboard().ClearAllKeys();
    ClearMouseButtons();

    hasFocus = e.stateEvent.state != 0;
    focusChanged.Emit(this, hasFocus);
}

void Window::HandleVisibilityChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_VISIBILITY_CHANGED: state=%s", e.stateEvent.state ? "visible" : "hidden");

    isVisible = e.stateEvent.state != 0;
    visibilityChanged.Emit(this, isVisible);
}

void Window::HandleMouseClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::MOUSE_BUTTON_DOWN;

    UIEvent uie;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.mouseButton = static_cast<UIEvent::MouseButton>(e.mouseEvent.button);

    // NOTE: Taken from CoreWin32Platform::OnMouseClick

    //bool isAnyButtonDownBefore = mouseButtonState.any();
    bool isButtonDown = uie.phase == UIEvent::Phase::BEGAN;
    uint32 buttonIndex = static_cast<uint32>(uie.mouseButton) - 1;
    mouseButtonState[buttonIndex] = isButtonDown;

    uiControlSystem->OnInput(&uie);

    //bool isAnyButtonDownAfter = mouseButtonState.any();
    //if (isAnyButtonDownBefore && !isAnyButtonDownAfter)
    //{
    //    ReleaseCapture();
    //}
    //else if (!isAnyButtonDownBefore && isAnyButtonDownAfter)
    //{
    //    SetCapture(hWindow);
    //}
}

void Window::HandleMouseWheel(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::WHEEL;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.wheelDelta = { e.mouseEvent.scrollDeltaX, e.mouseEvent.scrollDeltaY };

    // TODO: let input system decide what to do when shift is pressed while wheelling
    // Now use implementation from current core
    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT))
    {
        using std::swap;
        swap(uie.wheelDelta.x, uie.wheelDelta.y);
    }

    uiControlSystem->OnInput(&uie);
}

void Window::HandleMouseMove(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::MOVE;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
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
                uiControlSystem->OnInput(&uie);
            }
        }
    }
    else
    {
        uiControlSystem->OnInput(&uie);
    }
}

void Window::HandleTouchClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::TOUCH_DOWN;

    UIEvent uie;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.physPoint = Vector2(e.touchEvent.x, e.touchEvent.y);
    uie.device = UIEvent::Device::TOUCH_SURFACE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.touchId = e.touchEvent.touchId;

    uiControlSystem->OnInput(&uie);
}

void Window::HandleTouchMove(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::DRAG;
    uie.physPoint = Vector2(e.touchEvent.x, e.touchEvent.y);
    uie.device = UIEvent::Device::TOUCH_SURFACE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.touchId = e.touchEvent.touchId;

    uiControlSystem->OnInput(&uie);
}

void Window::HandleKeyPress(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::KEY_DOWN;

    KeyboardDevice& keyboard = inputSystem->GetKeyboard();

    UIEvent uie;
    uie.key = keyboard.GetDavaKeyForSystemKey(e.keyEvent.key);
    uie.device = UIEvent::Device::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    if (pressed)
    {
        uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::KEY_DOWN_REPEAT : UIEvent::Phase::KEY_DOWN;
    }
    else
    {
        uie.phase = UIEvent::Phase::KEY_UP;
    }

    uiControlSystem->OnInput(&uie);
    if (pressed)
    {
        keyboard.OnKeyPressed(uie.key);
    }
    else
    {
        keyboard.OnKeyUnpressed(uie.key);
    }
}

void Window::HandleKeyChar(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.keyChar = static_cast<char32_t>(e.keyEvent.key);
    uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::CHAR_REPEAT : UIEvent::Phase::CHAR;
    uie.device = UIEvent::Device::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    uiControlSystem->OnInput(&uie);
}

void Window::ClearMouseButtons()
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
            uiControlSystem->OnInput(&uie);
        }
    }
    mouseButtonState.reset();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
