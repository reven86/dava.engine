#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Window.h"

#include "Engine/Public/EngineContext.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/NativeWindow.h"

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
    delete nativeWindow;
    nativeWindow = nullptr;
}

void Window::Resize(float32 w, float32 h)
{
    if (nativeWindow != nullptr)
    {
        nativeWindow->Resize(w, h);
    }
}

void Window::Close()
{
    DVASSERT(nativeWindow != nullptr);
    nativeWindow->Close();
}

Engine* Window::GetEngine() const
{
    return engineBackend->GetEngine();
}

void* Window::GetNativeHandle() const
{
    DVASSERT(nativeWindow != nullptr);
    return nativeWindow->GetHandle();
}

WindowNativeService* Window::GetNativeService() const
{
    DVASSERT(nativeWindow != nullptr);
    return nativeWindow->GetNativeService();
}

void Window::RunAsyncOnUIThread(const Function<void()>& task)
{
    DVASSERT(nativeWindow != nullptr);
    nativeWindow->RunAsyncOnUIThread(task);
}

void Window::Update(float32 frameDelta)
{
    if (nativeWindow != nullptr)
    {
        uiControlSystem->Update();
        update.Emit(*this, frameDelta);
    }
}

void Window::Draw()
{
    if (nativeWindow != nullptr && isVisible)
    {
        uiControlSystem->Draw();
    }
}

void Window::PostFocusChanged(bool focus)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = DispatcherEvent::WINDOW_FOCUS_CHANGED;
    e.stateEvent.state = focus;
    dispatcher->PostEvent(e);
}

void Window::PostVisibilityChanged(bool visibility)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = DispatcherEvent::WINDOW_VISIBILITY_CHANGED;
    e.stateEvent.state = visibility;
    dispatcher->PostEvent(e);
}

void Window::PostSizeChanged(float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = DispatcherEvent::WINDOW_SIZE_SCALE_CHANGED;

    e.sizeEvent.width = width;
    e.sizeEvent.height = height;
    e.sizeEvent.scaleX = scaleX;
    e.sizeEvent.scaleY = scaleY;
    dispatcher->PostEvent(e);
}

void Window::PostWindowCreated(Private::NativeWindow* native, float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = DispatcherEvent::WINDOW_CREATED;

    e.windowCreatedEvent.nativeWindow = native;
    e.windowCreatedEvent.size.width = width;
    e.windowCreatedEvent.size.height = height;
    e.windowCreatedEvent.size.scaleX = scaleX;
    e.windowCreatedEvent.size.scaleY = scaleY;
    dispatcher->PostEvent(e);
}

void Window::PostWindowDestroyed()
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.type = DispatcherEvent::WINDOW_DESTROYED;
    dispatcher->PostEvent(e);
}

void Window::PostKeyDown(uint32 key, bool isRepeated)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.type = DispatcherEvent::KEY_DOWN;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
}

void Window::PostKeyUp(uint32 key)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.type = DispatcherEvent::KEY_UP;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = false;
    dispatcher->PostEvent(e);
}

void Window::PostKeyChar(uint32 key, bool isRepeated)
{
    using Private::DispatcherEvent;

    DispatcherEvent e;
    e.type = DispatcherEvent::KEY_CHAR;
    e.window = this;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = isRepeated;
    dispatcher->PostEvent(e);
}

void Window::EventHandler(const Private::DispatcherEvent& e)
{
    using Private::DispatcherEvent;
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
    case DispatcherEvent::WINDOW_SIZE_SCALE_CHANGED:
        HandleSizeChanged(e);
        break;
    case DispatcherEvent::WINDOW_FOCUS_CHANGED:
        HandleFocusChanged(e);
        break;
    case DispatcherEvent::WINDOW_VISIBILITY_CHANGED:
        HandleVisibilityChanged(e);
        break;
    case DispatcherEvent::WINDOW_CREATED:
        HandleWindowCreated(e);
        break;
    case DispatcherEvent::WINDOW_DESTROYED:
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

    if (nativeWindow != nullptr)
    {
        nativeWindow->TriggerPlatformEvents();
    }
}

void Window::HandleWindowCreated(const Private::DispatcherEvent& e)
{
    Logger::Error("****** WINDOW_CREATED: this=%p, w=%.1f, h=%.1f", this, e.windowCreatedEvent.size.width, e.windowCreatedEvent.size.height);

    nativeWindow = e.windowCreatedEvent.nativeWindow;

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

void Window::HandleWindowDestroyed(const Private::DispatcherEvent& e)
{
    Logger::Error("****** WINDOW_DESTROYED: this=%p", this);

    destroyed.Emit(*this);

    inputSystem = nullptr;
    uiControlSystem = nullptr;
    virtualCoordSystem = nullptr;

    engineBackend->DeinitRender(this);
}

void Window::HandleSizeChanged(const Private::DispatcherEvent& e)
{
    width = e.sizeEvent.width;
    height = e.sizeEvent.height;
    scaleX = e.sizeEvent.scaleX;
    scaleY = e.sizeEvent.scaleY;
    pendingSizeChanging = true;
}

void Window::HandleFocusChanged(const Private::DispatcherEvent& e)
{
    Logger::Error("****** WINDOW_FOCUS_CHANGED: this=%p, state=%u", this, e.stateEvent.state);

    inputSystem->GetKeyboard().ClearAllKeys();
    ClearMouseButtons();

    hasFocus = e.stateEvent.state != 0;
    focusChanged.Emit(*this, hasFocus);
}

void Window::HandleVisibilityChanged(const Private::DispatcherEvent& e)
{
    Logger::Error("****** WINDOW_VISIBILITY_CHANGED: this=%p, state=%u", this, e.stateEvent.state);

    isVisible = e.stateEvent.state != 0;
    visibilityChanged.Emit(*this, isVisible);
}

void Window::HandleMouseClick(const Private::DispatcherEvent& e)
{
    bool pressed = e.type == Private::DispatcherEvent::MOUSE_BUTTON_DOWN;

    Logger::Debug("****** %s: this=%p, x=%.1f, y=%.1f, button=%d", pressed ? "MOUSE_BUTTON_DOWN" : "MOUSE_BUTTON_UP", this, e.mclickEvent.x, e.mclickEvent.y, e.mclickEvent.button);

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

void Window::HandleMouseWheel(const Private::DispatcherEvent& e)
{
    Logger::Debug("****** MOUSE_WHEEL: this=%p, x=%.1f, y=%.1f, delta=%d", this, e.mwheelEvent.x, e.mwheelEvent.y, e.mwheelEvent.delta);

    UIEvent uie;
    uie.phase = UIEvent::Phase::WHEEL;
    uie.physPoint = Vector2(e.mwheelEvent.x, e.mwheelEvent.y);
    uie.device = UIEvent::Device::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.wheelDelta = { 0.0f, static_cast<float32>(e.mwheelEvent.delta) };

    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT))
    {
        using std::swap;
        swap(uie.wheelDelta.x, uie.wheelDelta.y);
    }

    uiControlSystem->OnInput(&uie);
}

void Window::HandleMouseMove(const Private::DispatcherEvent& e)
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

void Window::HandleKeyPress(const Private::DispatcherEvent& e)
{
    bool pressed = e.type == Private::DispatcherEvent::KEY_DOWN;

    Logger::Debug("****** %s: this=%p", pressed ? "KEY_DOWN" : "KEY_UP", this);

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

void Window::HandleKeyChar(const Private::DispatcherEvent& e)
{
    Logger::Debug("****** KEY_CHAR: this=%p", this);

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
        Logger::Debug("****** WindowBackend init renderer: this=%p, w=%d, h=%d, pw=%d, ph=%d", this, w, h, physW, physH);

        engineBackend->InitRenderer(this);
        pendingInitRender = false;
    }
    else
    {
        Logger::Debug("****** WindowBackend reset renderer: this=%p, w=%d, h=%d, pw=%d, ph=%d", this, w, h, physW, physH);

        engineBackend->ResetRenderer(this, false);
    }

    virtualCoordSystem->SetInputScreenAreaSize(w, h);
    virtualCoordSystem->SetPhysicalScreenSize(physW, physH);
    virtualCoordSystem->UnregisterAllAvailableResourceSizes();
    virtualCoordSystem->RegisterAvailableResourceSize(w, h, "Gfx");
    virtualCoordSystem->ScreenSizeChanged();

    sizeScaleChanged.Emit(*this, width, height, scaleX, scaleY);
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
