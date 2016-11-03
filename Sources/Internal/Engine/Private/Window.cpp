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

void Window::SetSize(Size2f sz)
{
    // Window cannot be resized in embedded mode as window lifetime
    // is controlled by highlevel framework
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->Resize(sz.dx, sz.dy);
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
    case MainDispatcherEvent::TRACKPAD_GESTURE:
        HandleTrackpadGesture(e);
        break;
    case MainDispatcherEvent::KEY_DOWN:
    case MainDispatcherEvent::KEY_UP:
        HandleKeyPress(e);
        break;
    case MainDispatcherEvent::KEY_CHAR:
        HandleKeyChar(e);
        break;
    case MainDispatcherEvent::WINDOW_SIZE_CHANGED:
        HandleSizeChanged(e);
        break;
    case MainDispatcherEvent::WINDOW_DPI_CHANGED:
        HandleDpiChanged(e);
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
    sizeEventsMerged = false;
    windowBackend->TriggerPlatformEvents();
}

void Window::HandleWindowCreated(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_CREATED, dpi %.1f", e.sizeEvent.dpi);

    dpi = e.sizeEvent.dpi;
    MergeSizeChangedEvents(e);
    sizeEventsMerged = true;

    engineBackend->InitRenderer(this);

    EngineContext* context = engineBackend->GetEngineContext();
    inputSystem = context->inputSystem;
    uiControlSystem = context->uiControlSystem;

    UpdateVirtualCoordinatesSystem();

    engineBackend->OnWindowCreated(this);
    sizeChanged.Emit(this, GetSize(), GetSurfaceSize());
}

void Window::HandleWindowDestroyed(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_DESTROYED");

    engineBackend->OnWindowDestroyed(this);

    inputSystem = nullptr;
    uiControlSystem = nullptr;

    engineBackend->DeinitRender(this);
}

void Window::HandleSizeChanged(const Private::MainDispatcherEvent& e)
{
    if (!sizeEventsMerged)
    {
        Logger::FrameworkDebug("=========== WINDOW_SIZE_CHANGED");

        MergeSizeChangedEvents(e);
        sizeEventsMerged = true;

        engineBackend->ResetRenderer(this, !windowBackend->IsWindowReadyForRender());
        if (windowBackend->IsWindowReadyForRender())
        {
            UpdateVirtualCoordinatesSystem();

            sizeChanged.Emit(this, GetSize(), GetSurfaceSize());
        }
    }
}

void Window::HandleDpiChanged(const Private::MainDispatcherEvent& e)
{
    dpi = e.dpiEvent.dpi;
    dpiChanged.Emit(this, dpi);
}

void Window::MergeSizeChangedEvents(const Private::MainDispatcherEvent& e)
{
    // Look into dispatcher queue and compress size events into one event to allow:
    //  - single render init/reset call during one frame
    //  - emit signals about window creation or size changing immediately on event receiving
    using Private::MainDispatcherEvent;
    MainDispatcherEvent::WindowSizeEvent compressedSize(e.sizeEvent);
    mainDispatcher->ViewEventQueue([this, &compressedSize](const MainDispatcherEvent& e) {
        if (e.window == this && e.type == MainDispatcherEvent::WINDOW_SIZE_CHANGED)
        {
            compressedSize = e.sizeEvent;
        }
    });

    width = compressedSize.width;
    height = compressedSize.height;
    surfaceWidth = compressedSize.surfaceWidth;
    surfaceHeight = compressedSize.surfaceHeight;

    Logger::FrameworkDebug("=========== SizeChanged merged to: width=%.1f, height=%.1f, surfaceW=%.3f, surfaceH=%.3f", width, height, surfaceWidth, surfaceHeight);
}

void Window::UpdateVirtualCoordinatesSystem()
{
    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);

    Size2f surfSize = GetSurfaceSize();

    int32 sw = static_cast<int32>(surfSize.dx);
    int32 sh = static_cast<int32>(surfSize.dy);

    uiControlSystem->vcs->SetInputScreenAreaSize(w, h);
    uiControlSystem->vcs->SetPhysicalScreenSize(sw, sh);
    uiControlSystem->vcs->UnregisterAllAvailableResourceSizes();
    uiControlSystem->vcs->RegisterAvailableResourceSize(w, h, "Gfx");
    uiControlSystem->vcs->ScreenSizeChanged();
}

void Window::HandleFocusChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_FOCUS_CHANGED: state=%s", e.stateEvent.state ? "got_focus" : "lost_focus");

    inputSystem->GetKeyboard().ClearAllKeys();

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
    eMouseButtons button = e.mouseEvent.button;

    UIEvent uie;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.device = eInputDevices::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.mouseButton = button;
    uie.modifiers = e.mouseEvent.modifierKeys;

    uint32 buttonIndex = static_cast<uint32>(button) - 1;
    mouseButtonState[buttonIndex] = pressed;

    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleMouseWheel(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::WHEEL;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.device = eInputDevices::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.wheelDelta = { e.mouseEvent.scrollDeltaX, e.mouseEvent.scrollDeltaY };
    uie.modifiers = e.mouseEvent.modifierKeys;

    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleMouseMove(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::MOVE;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.device = eInputDevices::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.mouseButton = eMouseButtons::NONE;
    uie.modifiers = e.mouseEvent.modifierKeys;

    if (mouseButtonState.any())
    {
        // Send DRAG phase instead of MOVE for each pressed mouse button
        uie.phase = UIEvent::Phase::DRAG;

        uint32 firstButton = static_cast<uint32>(eMouseButtons::FIRST);
        uint32 lastButton = static_cast<uint32>(eMouseButtons::LAST);
        for (uint32 buttonIndex = firstButton; buttonIndex <= lastButton; ++buttonIndex)
        {
            if (mouseButtonState[buttonIndex - 1])
            {
                uie.mouseButton = static_cast<eMouseButtons>(buttonIndex);
                inputSystem->HandleInputEvent(&uie);
            }
        }
    }
    else
    {
        inputSystem->HandleInputEvent(&uie);
    }
}

void Window::HandleTouchClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::TOUCH_DOWN;

    UIEvent uie;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.physPoint = Vector2(e.touchEvent.x, e.touchEvent.y);
    uie.device = eInputDevices::TOUCH_SURFACE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.touchId = e.touchEvent.touchId;
    uie.modifiers = e.touchEvent.modifierKeys;

    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleTouchMove(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.phase = UIEvent::Phase::DRAG;
    uie.physPoint = Vector2(e.touchEvent.x, e.touchEvent.y);
    uie.device = eInputDevices::TOUCH_SURFACE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.touchId = e.touchEvent.touchId;
    uie.modifiers = e.touchEvent.modifierKeys;

    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleTrackpadGesture(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.timestamp = e.timestamp / 1000.0;
    uie.modifiers = e.trackpadGestureEvent.modifierKeys;
    uie.device = eInputDevices::TOUCH_PAD;
    uie.phase = UIEvent::Phase::GESTURE;
    uie.gesture.magnification = e.trackpadGestureEvent.magnification;
    uie.gesture.rotation = e.trackpadGestureEvent.rotation;
    uie.gesture.dx = e.trackpadGestureEvent.deltaX;
    uie.gesture.dy = e.trackpadGestureEvent.deltaY;

    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleKeyPress(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::KEY_DOWN;

    KeyboardDevice& keyboard = inputSystem->GetKeyboard();

    UIEvent uie;
    uie.key = keyboard.GetDavaKeyForSystemKey(e.keyEvent.key);
    uie.device = eInputDevices::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;
    uie.modifiers = e.keyEvent.modifierKeys;

    if (pressed)
    {
        uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::KEY_DOWN_REPEAT : UIEvent::Phase::KEY_DOWN;
    }
    else
    {
        uie.phase = UIEvent::Phase::KEY_UP;
    }

    inputSystem->HandleInputEvent(&uie);
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
    uie.device = eInputDevices::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;
    uie.modifiers = e.keyEvent.modifierKeys;

    inputSystem->HandleInputEvent(&uie);
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
