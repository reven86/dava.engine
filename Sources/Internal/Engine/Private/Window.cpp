#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Window.h"

#include "Engine/Public/EngineContext.h"
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
Window::Window(Private::EngineBackend* engine, bool primary)
    : engineBackend(engine)
    , dispatcher(engineBackend->GetDispatcher())
    , isPrimary(primary)
{
}

Window::~Window()
{
    delete windowBackend;
    windowBackend = nullptr;
}

void Window::Resize(float32 w, float32 h)
{
    if (windowBackend != nullptr)
    {
        windowBackend->Resize(w, h);
    }
}

void Window::Close()
{
    DVASSERT(windowBackend != nullptr);
    windowBackend->Close();
}

Engine* Window::GetEngine() const
{
    return engineBackend->GetEngine();
}

void* Window::GetNativeHandle() const
{
    DVASSERT(windowBackend != nullptr);
    return windowBackend->GetHandle();
}

WindowNativeService* Window::GetNativeService() const
{
    DVASSERT(windowBackend != nullptr);
    return windowBackend->GetNativeService();
}

void Window::RunAsyncOnUIThread(const Function<void()>& task)
{
    DVASSERT(windowBackend != nullptr);
    windowBackend->RunAsyncOnUIThread(task);
}

void Window::Update(float32 frameDelta)
{
    if (windowBackend != nullptr)
    {
        uiControlSystem->Update();
        update.Emit(*this, frameDelta);
    }
}

void Window::Draw()
{
    if (windowBackend != nullptr && isVisible)
    {
        uiControlSystem->Draw();
    }
}

void Window::PostFocusChanged(bool focus)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = MainDispatcherEvent::WINDOW_FOCUS_CHANGED;
    e.stateEvent.state = focus;
    dispatcher->PostEvent(e);
}

void Window::PostVisibilityChanged(bool visibility)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = MainDispatcherEvent::WINDOW_VISIBILITY_CHANGED;
    e.stateEvent.state = visibility;
    dispatcher->PostEvent(e);
}

void Window::PostSizeChanged(float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = MainDispatcherEvent::WINDOW_SIZE_SCALE_CHANGED;

    e.sizeEvent.width = width;
    e.sizeEvent.height = height;
    e.sizeEvent.scaleX = scaleX;
    e.sizeEvent.scaleY = scaleY;
    dispatcher->PostEvent(e);
}

void Window::PostWindowCreated(Private::WindowBackend* wbackend, float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = MainDispatcherEvent::WINDOW_CREATED;

    e.windowCreatedEvent.windowBackend = wbackend;
    e.windowCreatedEvent.size.width = width;
    e.windowCreatedEvent.size.height = height;
    e.windowCreatedEvent.size.scaleX = scaleX;
    e.windowCreatedEvent.size.scaleY = scaleY;
    dispatcher->PostEvent(e);
}

void Window::PostWindowDestroyed()
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = MainDispatcherEvent::WINDOW_DESTROYED;
    dispatcher->PostEvent(e);
}

void Window::PostKeyDown(uint32 key, bool isRepeated)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::KEY_DOWN;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
}

void Window::PostKeyUp(uint32 key)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::KEY_UP;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = false;
    dispatcher->PostEvent(e);
}

void Window::PostKeyChar(uint32 key, bool isRepeated)
{
    using Private::MainDispatcherEvent;

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::KEY_CHAR;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
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
    if (pendingSizeChanging)
    {
        HandlePendingSizeChanging();
        pendingSizeChanging = false;
    }

    if (windowBackend != nullptr)
    {
        windowBackend->TriggerPlatformEvents();
    }
}

void Window::HandleWindowCreated(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_CREATED: w=%.1f, h=%.1f", e.windowCreatedEvent.size.width, e.windowCreatedEvent.size.height);

    windowBackend = e.windowCreatedEvent.windowBackend;

    width = e.windowCreatedEvent.size.width;
    height = e.windowCreatedEvent.size.height;
    scaleX = e.windowCreatedEvent.size.scaleX;
    scaleY = e.windowCreatedEvent.size.scaleY;

    pendingInitRender = true;
    pendingSizeChanging = true;

    EngineContext* context = engineBackend->GetEngineContext();
    inputSystem = context->inputSystem;
    uiControlSystem = context->uiControlSystem;
    virtualCoordSystem = context->virtualCoordSystem;

    virtualCoordSystem->EnableReloadResourceOnResize(true);
}

void Window::HandleWindowDestroyed(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_DESTROYED");

    destroyed.Emit(*this);

    inputSystem = nullptr;
    uiControlSystem = nullptr;
    virtualCoordSystem = nullptr;

    engineBackend->DeinitRender(this);
}

void Window::HandleSizeChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_SIZE_SCALE_CHANGED: w=%.1f, h=%.1f", e.sizeEvent.width, e.sizeEvent.height);

    width = e.sizeEvent.width;
    height = e.sizeEvent.height;
    scaleX = e.sizeEvent.scaleX;
    scaleY = e.sizeEvent.scaleY;
    pendingSizeChanging = true;
}

void Window::HandleFocusChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_FOCUS_CHANGED: state=%u", e.stateEvent.state ? "got_focus" : "lost_focus");

    inputSystem->GetKeyboard().ClearAllKeys();
    ClearMouseButtons();

    hasFocus = e.stateEvent.state != 0;
    focusChanged.Emit(*this, hasFocus);
}

void Window::HandleVisibilityChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_VISIBILITY_CHANGED: state=%s", e.stateEvent.state ? "visible" : "hidden");

    isVisible = e.stateEvent.state != 0;
    visibilityChanged.Emit(*this, isVisible);
}

void Window::HandleMouseClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::MOUSE_BUTTON_DOWN;

    UIEvent uie;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.physPoint = Vector2(e.mclickEvent.x, e.mclickEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.mouseButton = static_cast<UIEvent::MouseButton>(e.mclickEvent.button);

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
    uie.physPoint = Vector2(e.mwheelEvent.x, e.mwheelEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.wheelDelta = { e.mwheelEvent.deltaX, e.mwheelEvent.deltaY };

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
    uie.physPoint = Vector2(e.tclickEvent.x, e.tclickEvent.y);
    uie.device = UIEvent::Device::TOUCH_SURFACE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.touchId = e.tclickEvent.touchId;

    uiControlSystem->OnInput(&uie);
}

void Window::HandleTouchMove(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::DRAG;
    uie.physPoint = Vector2(e.tclickEvent.x, e.tclickEvent.y);
    uie.device = UIEvent::Device::TOUCH_SURFACE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.touchId = e.tclickEvent.touchId;

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

void Window::HandlePendingSizeChanging()
{
    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);
    int32 physW = static_cast<int32>(GetRenderSurfaceWidth());
    int32 physH = static_cast<int32>(GetRenderSurfaceHeight());

    if (pendingInitRender)
    {
        engineBackend->InitRenderer(this);
        pendingInitRender = false;
    }
    else
    {
        engineBackend->ResetRenderer(this, !windowBackend->IsWindowReadyForRender());
    }

    if (windowBackend->IsWindowReadyForRender())
    {
        virtualCoordSystem->SetInputScreenAreaSize(w, h);
        virtualCoordSystem->SetPhysicalScreenSize(physW, physH);
        virtualCoordSystem->UnregisterAllAvailableResourceSizes();
        virtualCoordSystem->RegisterAvailableResourceSize(w, h, "Gfx");
        virtualCoordSystem->ScreenSizeChanged();

        sizeScaleChanged.Emit(*this, width, height, scaleX, scaleY);
    }
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
