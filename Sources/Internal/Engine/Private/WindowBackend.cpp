#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Window.h"

#include "Engine/Public/AppContext.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/NativeWindow.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/DispatcherEvent.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"
#include "Input/InputSystem.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
namespace Private
{
WindowBackend::WindowBackend(EngineBackend* e, bool primary)
    : window(new Window(this))
    , engineBackend(e)
    , isPrimary(primary)
{
}

WindowBackend::~WindowBackend()
{
    delete window;
    window = nullptr;
}

void WindowBackend::Resize(float32 w, float32 h)
{
    if (nativeWindow != nullptr)
    {
        nativeWindow->Resize(w, h);
    }
    else
    {
        pendingWidth = w;
        pendingHeight = h;
        pendingResizeRequest = true;
    }
}

void* WindowBackend::GetNativeHandle() const
{
    DVASSERT(nativeWindow != nullptr);
    return nativeWindow->GetHandle();
}

void WindowBackend::RunAsyncOnUIThread(const Function<void()>& task)
{
    DVASSERT(nativeWindow != nullptr);
    nativeWindow->RunAsyncOnUIThread(task);
}

void WindowBackend::EventHandler(const DispatcherEvent& e)
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
    }
}

void WindowBackend::FinishEventHandlingOnCurrentFrame()
{
    if (pendingSizeChanging)
    {
        HandlePendingSizeChanging();
        pendingSizeChanging = false;

        Logger::Debug("****** WindowBackend::FinishEventHandlingOnCurrentFrame");
    }
}

void WindowBackend::Update(float32 frameDelta)
{
    if (nativeWindow != nullptr)
    {
        uiControlSystem->Update();
        window->update.Emit(window, frameDelta);
    }
}

void WindowBackend::Draw()
{
    if (nativeWindow != nullptr && isVisible)
    {
        uiControlSystem->Draw();
    }
}

void WindowBackend::HandleWindowCreated(const DispatcherEvent& e)
{
    inputSystem = engineBackend->context->inputSystem;
    uiControlSystem = engineBackend->context->uiControlSystem;
    virtualCoordSystem = engineBackend->context->virtualCoordSystem;

    Logger::Error("****** WINDOW_CREATED: w=%.1f, h=%.1f", e.windowCreatedEvent.size.width, e.windowCreatedEvent.size.width);

    nativeWindow = e.windowCreatedEvent.nativeWindow;

    width = e.windowCreatedEvent.size.width;
    height = e.windowCreatedEvent.size.height;
    scaleX = e.windowCreatedEvent.size.scaleX;
    scaleY = e.windowCreatedEvent.size.scaleY;

    virtualCoordSystem->EnableReloadResourceOnResize(true);

    pendingInitRender = true;
    pendingSizeChanging = true;

    if (pendingResizeRequest)
    {
        nativeWindow->Resize(pendingWidth, pendingHeight);
        pendingResizeRequest = false;
    }
}

void WindowBackend::HandleWindowDestroyed(const DispatcherEvent& e)
{
    nativeWindow = nullptr;

    inputSystem = nullptr;
    uiControlSystem = nullptr;
    virtualCoordSystem = nullptr;

    engineBackend->DeinitRender(this);
}

void WindowBackend::HandleSizeChanged(const DispatcherEvent& e)
{
    width = e.sizeEvent.width;
    height = e.sizeEvent.height;
    scaleX = e.sizeEvent.scaleX;
    scaleY = e.sizeEvent.scaleY;
    pendingSizeChanging = true;
}

void WindowBackend::HandleFocusChanged(const DispatcherEvent& e)
{
    Logger::Error("****** WINDOW_FOCUS_CHANGED: state=%u", e.stateEvent.state);

    inputSystem->GetKeyboard().ClearAllKeys();
    ClearMouseButtons();

    hasFocus = e.stateEvent.state != 0;
    window->focusChanged.Emit(window, hasFocus);
}

void WindowBackend::HandleVisibilityChanged(const DispatcherEvent& e)
{
    Logger::Error("****** WINDOW_VISIBILITY_CHANGED: state=%u", e.stateEvent.state);

    isVisible = e.stateEvent.state != 0;
    window->visibilityChanged.Emit(window, isVisible);
}

void WindowBackend::HandleMouseClick(const DispatcherEvent& e)
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

    uiControlSystem->OnInput(&uie);

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

void WindowBackend::HandleMouseWheel(const DispatcherEvent& e)
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
        using std::swap;
        swap(uie.wheelDelta.x, uie.wheelDelta.y);
    }

    uiControlSystem->OnInput(&uie);
}

void WindowBackend::HandleMouseMove(const DispatcherEvent& e)
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
                uiControlSystem->OnInput(&uie);
            }
        }
    }
    else
    {
        uiControlSystem->OnInput(&uie);
    }
}

void WindowBackend::HandleKeyPress(const DispatcherEvent& e)
{
    bool pressed = e.type == DispatcherEvent::KEY_DOWN;

    Logger::Debug("****** %s", pressed ? "KEY_DOWN" : "KEY_UP");

    KeyboardDevice& keyboard = inputSystem->GetKeyboard();

    UIEvent uie;
    uie.key = keyboard.GetDavaKeyForSystemKey(e.keyEvent.key);
    uie.device = UIEvent::Device::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    if (pressed)
        uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::KEY_DOWN_REPEAT : UIEvent::Phase::KEY_DOWN;
    else
        uie.phase = UIEvent::Phase::KEY_UP;

    uiControlSystem->OnInput(&uie);
    keyboard.OnKeyPressed(uie.key);
}

void WindowBackend::HandleKeyChar(const DispatcherEvent& e)
{
    Logger::Debug("****** KEY_CHAR");

    UIEvent uie;
    uie.keyChar = static_cast<char32_t>(e.keyEvent.key);
    uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::CHAR_REPEAT : UIEvent::Phase::CHAR;
    uie.device = UIEvent::Device::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;

    uiControlSystem->OnInput(&uie);
}

void WindowBackend::HandlePendingSizeChanging()
{
    if (pendingInitRender)
    {
        Logger::Debug("****** WindowBackend: init renderer");

        engineBackend->InitRenderer(this);
        pendingInitRender = false;
    }
    else
    {
        Logger::Debug("****** WindowBackend: reset renderer");

        engineBackend->ResetRenderer(this, false);
    }

    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);
    virtualCoordSystem->SetInputScreenAreaSize(w, h);
    virtualCoordSystem->SetPhysicalScreenSize(w, h);
    virtualCoordSystem->UnregisterAllAvailableResourceSizes();
    virtualCoordSystem->RegisterAvailableResourceSize(w, h, "Gfx");
    virtualCoordSystem->ScreenSizeChanged();

    window->sizeScaleChanged.Emit(window, width, height, scaleX, scaleY);
}

void WindowBackend::ClearMouseButtons()
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

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
