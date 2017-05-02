#include "Engine/Window.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/EngineContext.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/WindowBackend.h"

#include "Utils/StringFormat.h"
#include "Animation/AnimationManager.h"
#include "Autotesting/AutotestingSystem.h"
#include "Input/InputSystem.h"
#include "Logger/Logger.h"
#include "Time/SystemTimer.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
Window::Window(Private::EngineBackend* engineBackend, bool primary)
    : engineBackend(engineBackend)
    , mainDispatcher(engineBackend->GetDispatcher())
    , windowBackend(new Private::WindowBackend(engineBackend, this))
    , isPrimary(primary)
{
    // TODO: Add platfom's caps check
    //if (windowBackend->IsPlatformSupported(SET_CURSOR_VISIBILITY))
    {
        cursorVisible = true;
    }
}

Window::~Window() = default;

void Window::SetSizeAsync(Size2f sz)
{
    // Window cannot be resized in embedded mode as window lifetime
    // is controlled by highlevel framework
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->Resize(sz.dx, sz.dy);
    }
}

void Window::SetMinimumSize(Size2f size)
{
    DVASSERT(size.dx >= 0.f && size.dy >= 0.f);

    if (!engineBackend->IsEmbeddedGUIMode())
    {
        size.dx = std::max(size.dx, static_cast<float32>(smallestWidth));
        size.dy = std::max(size.dy, static_cast<float32>(smallestHeight));

        windowBackend->SetMinimumSize(size);
    }
}

void Window::SetVirtualSize(float32 w, float32 h)
{
    uiControlSystem->vcs->SetVirtualScreenSize(static_cast<int32>(w), static_cast<int32>(h));
}

Size2f Window::GetVirtualSize() const
{
    Size2i sz = uiControlSystem->vcs->GetVirtualScreenSize();
    return Size2f(static_cast<float32>(sz.dx), static_cast<float32>(sz.dy));
}

void Window::CloseAsync()
{
    // Window cannot be close in embedded mode as window lifetime
    // is controlled by highlevel framework
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->Close(false);
    }
}

void Window::SetTitleAsync(const String& title)
{
    // It does not make sense to set window title in embedded mode
    if (!engineBackend->IsEmbeddedGUIMode())
    {
        windowBackend->SetTitle(title);
    }
}

void Window::SetFullscreenAsync(eFullscreen newMode)
{
    // Window's fullscreen mode cannot be changed in embedded mode
    if (!engineBackend->IsEmbeddedGUIMode() && newMode != fullscreenMode)
    {
        windowBackend->SetFullscreen(newMode);
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

void Window::RunOnUIThreadAsync(const Function<void()>& task)
{
    windowBackend->RunAsyncOnUIThread(task);
}

void Window::RunOnUIThread(const Function<void()>& task)
{
    windowBackend->RunAndWaitOnUIThread(task);
}

void Window::InitCustomRenderParams(rhi::InitParam& params)
{
    windowBackend->InitCustomRenderParams(params);
}

void Window::SetCursorCapture(eCursorCapture mode)
{
    // For now FRAME is not supported, introduce somehow later
    if (mode == eCursorCapture::FRAME)
        return;

    /*if (windowBackend->IsPlatformSupported(SET_CURSOR_CAPTURE))*/ // TODO: Add platfom's caps check
    {
        if (cursorCapture != mode)
        {
            cursorCapture = mode;
            if (cursorCapture == eCursorCapture::PINNING)
            {
                waitInputActivation |= !hasFocus;
                if (!waitInputActivation)
                {
                    windowBackend->SetCursorCapture(cursorCapture);
                    windowBackend->SetCursorVisibility(false);
                }
            }
            else if (hasFocus)
            {
                windowBackend->SetCursorCapture(cursorCapture);
                windowBackend->SetCursorVisibility(cursorVisible);
            }
        }
    }
}

eCursorCapture Window::GetCursorCapture() const
{
    return cursorCapture;
}

void Window::SetCursorVisibility(bool visible)
{
    /*if (windowBackend->IsPlatformSupported(SET_CURSOR_VISIBILITY))*/ // TODO: Add platfom's caps check
    {
        if (cursorVisible != visible)
        {
            cursorVisible = visible;
            if (hasFocus && cursorCapture != eCursorCapture::PINNING)
            {
                windowBackend->SetCursorVisibility(cursorVisible);
            }
        }
    }
}

bool Window::GetCursorVisibility() const
{
    return cursorVisible && cursorCapture != eCursorCapture::PINNING;
}

void Window::Update(float32 frameDelta)
{
    update.Emit(this, frameDelta);

    const EngineContext* context = engineBackend->GetContext();

#if defined(__DAVAENGINE_AUTOTESTING__)
    float32 realFrameDelta = SystemTimer::GetRealFrameDelta();
    context->autotestingSystem->Update(realFrameDelta);
#endif

    context->animationManager->Update(frameDelta);

    uiControlSystem->Update();
}

void Window::Draw()
{
    const EngineContext* context = engineBackend->GetContext();
    context->renderSystem2D->BeginFrame();

    uiControlSystem->Draw();

#if defined(__DAVAENGINE_AUTOTESTING__)
    context->autotestingSystem->Draw();
#endif
    draw.Emit(this);

    context->renderSystem2D->EndFrame();
}

void Window::EventHandler(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;
    if (MainDispatcherEvent::IsInputEvent(e.type))
    {
        // Skip input events if window does not have focus or pinning switching logic tells to ignore input event
        if (!hasFocus || HandleInputActivation(e))
        {
            return;
        }
    }
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
    case MainDispatcherEvent::WINDOW_CAPTURE_LOST:
        HandleCursorCaptureLost(e);
        break;
    case MainDispatcherEvent::WINDOW_CANCEL_INPUT:
        HandleCancelInput(e);
        break;
    case MainDispatcherEvent::WINDOW_VISIBLE_FRAME_CHANGED:
        HandleVisibleFrameChanged(e);
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
    Logger::Info("Window::HandleWindowCreated: enter");

    isAlive = true;
    MergeSizeChangedEvents(e);
    sizeEventsMerged = true;

    engineBackend->InitRenderer(this);

    const EngineContext* context = engineBackend->GetContext();
    inputSystem = context->inputSystem;
    uiControlSystem = context->uiControlSystem;

    UpdateVirtualCoordinatesSystem();
    engineBackend->OnWindowCreated(this);

    sizeChanged.Emit(this, GetSize(), GetSurfaceSize());

    Logger::Info("Window::HandleWindowCreated: leave");
}

void Window::HandleWindowDestroyed(const Private::MainDispatcherEvent& e)
{
    Logger::Info("Window::HandleWindowDestroyed: enter");

    engineBackend->OnWindowDestroyed(this);

    inputSystem = nullptr;
    uiControlSystem = nullptr;

    engineBackend->DeinitRender(this);
    isAlive = false;

    Logger::Info("Window::HandleWindowDestroyed: leave");
}

void Window::HandleCursorCaptureLost(const Private::MainDispatcherEvent& e)
{
    // If the native window loses the cursor capture, restore it and visibility when input activated.
    waitInputActivation = true;
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

            // TODO:
            // Resources must be separated from VirtualCoordinateSystem
            // Each resource consumer have to care for his resources by itself,
            // e.g. sprites reloading mechanism should be implemented in Sprite.cpp
            // by handling Window::sizeChanged signal and making sprites reload
            // inside that particular handler...
            //
            // Unfortunately we have only temporary solution:
            // call reloadig sprites/fonts from this point ((
            if (uiControlSystem->vcs->GetReloadResourceOnResize())
            {
// Disable sprite reloading on macos and windows
// Game uses separate thread for loading battle and its resources.
// Window resizing during battle loading may lead to crash as sprite
// reloading is not ready for multiple threads.
// TODO: do something with sprite reloading
//
// !!! At the moment this is a huge architectural problem,
// that we do not know how to solve.
// More detail can be found in DF-13044
//
#if !defined(__DAVAENGINE_MACOS__) && !defined(__DAVAENGINE_WINDOWS__)
                Sprite::ValidateForSize();
#endif
            }
        }
    }
}

void Window::HandleDpiChanged(const Private::MainDispatcherEvent& e)
{
    Logger::FrameworkDebug("=========== WINDOW_DPI_CHANGED: dpi=%f", e.dpiEvent.dpi);

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
    surfaceScale = compressedSize.surfaceScale;
    fullscreenMode = compressedSize.fullscreen;
    dpi = compressedSize.dpi;

    Logger::FrameworkDebug("=========== SizeChanged merged to: width=%.1f, height=%.1f, surfaceW=%.3f, surfaceH=%.3f, dpi=%f", width, height, surfaceWidth, surfaceHeight, dpi);
}

void Window::UpdateVirtualCoordinatesSystem()
{
    int32 w = static_cast<int32>(width);
    int32 h = static_cast<int32>(height);

    int32 sw = static_cast<int32>(surfaceWidth);
    int32 sh = static_cast<int32>(surfaceHeight);

    uiControlSystem->vcs->SetInputScreenAreaSize(w, h);
    uiControlSystem->vcs->SetPhysicalScreenSize(sw, sh);
}

bool Window::HandleInputActivation(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;
    if (waitInputActivation && cursorCapture == eCursorCapture::PINNING)
    {
        bool skipEvent = true;
        bool enablePinning = true;
        switch (e.type)
        {
        case MainDispatcherEvent::MOUSE_BUTTON_DOWN:
            skipEvent = true;
            enablePinning = true;
            break;
        case MainDispatcherEvent::MOUSE_BUTTON_UP:
            skipEvent = true;
            enablePinning = true;
            waitInputActivation = false;
            break;
        case MainDispatcherEvent::MOUSE_MOVE:
            skipEvent = true;
            enablePinning = false;
            break;
        default:
            skipEvent = false;
            enablePinning = true;
            waitInputActivation = false;
            break;
        }

        if (enablePinning)
        {
            windowBackend->SetCursorCapture(eCursorCapture::PINNING);
            windowBackend->SetCursorVisibility(false);
        }
        return skipEvent;
    }
    waitInputActivation = false;
    return false;
}

void Window::HandleCancelInput(const Private::MainDispatcherEvent& e)
{
    uiControlSystem->CancelAllInputs();
    inputSystem->GetKeyboard().ClearAllKeys();
}

void Window::HandleVisibleFrameChanged(const Private::MainDispatcherEvent& e)
{
    Rect visibleRect(e.visibleFrameEvent.x, e.visibleFrameEvent.y, e.visibleFrameEvent.width, e.visibleFrameEvent.height);
    visibleFrameChanged.Emit(this, visibleRect);
}

void Window::HandleFocusChanged(const Private::MainDispatcherEvent& e)
{
    bool gainsFocus = e.stateEvent.state != 0;
    if (hasFocus != gainsFocus)
    {
        Logger::FrameworkDebug("=========== WINDOW_FOCUS_CHANGED: state=%s", e.stateEvent.state ? "got_focus" : "lost_focus");

        uiControlSystem->CancelAllInputs();
        inputSystem->GetKeyboard().ClearAllKeys();
        hasFocus = gainsFocus;
        /*if (windowBackend->IsPlatformSupported(SET_CURSOR_CAPTURE))*/ // TODO: Add platfom's caps check
        {
            // When the native window loses focus, it restores the original cursor capture and visibility.
            // After the window gives the focus back, set the current visibility state, if not set pinning mode.
            // If the cursor capture mode is pinning, set the visibility state and capture mode when input activated.
            if (hasFocus && cursorCapture != eCursorCapture::PINNING)
            {
                windowBackend->SetCursorVisibility(cursorVisible);
                windowBackend->SetCursorCapture(cursorCapture);
            }
        }
        focusChanged.Emit(this, hasFocus);
    }
}

void Window::HandleVisibilityChanged(const Private::MainDispatcherEvent& e)
{
    bool becomesVisible = e.stateEvent.state != 0;
    if (isVisible != becomesVisible)
    {
        Logger::Info("Window::HandleVisibilityChanged: become %s", e.stateEvent.state ? "visible" : "hidden");

        isVisible = becomesVisible;
        visibilityChanged.Emit(this, isVisible);

        waitInputActivation = isVisible;
    }
}

void Window::HandleMouseClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::MOUSE_BUTTON_DOWN;
    eMouseButtons button = e.mouseEvent.button;

    UIEvent uie;
    uie.window = e.window;
    uie.phase = pressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
    uie.isRelative = e.mouseEvent.isRelative;
    uie.physPoint = e.mouseEvent.isRelative ? Vector2(0.f, 0.f) : Vector2(e.mouseEvent.x, e.mouseEvent.y);
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
    uie.window = e.window;
    uie.phase = UIEvent::Phase::WHEEL;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.isRelative = e.mouseEvent.isRelative;
    uie.device = eInputDevices::MOUSE;
    uie.timestamp = e.timestamp / 1000.0;
    uie.wheelDelta = { e.mouseEvent.scrollDeltaX, e.mouseEvent.scrollDeltaY };
    uie.modifiers = e.mouseEvent.modifierKeys;

    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleMouseMove(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.window = e.window;
    uie.phase = UIEvent::Phase::MOVE;
    uie.physPoint = Vector2(e.mouseEvent.x, e.mouseEvent.y);
    uie.isRelative = e.mouseEvent.isRelative;
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
    uie.window = e.window;
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
    uie.window = e.window;
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
    uie.window = e.window;
    uie.timestamp = e.timestamp / 1000.0;
    uie.modifiers = e.trackpadGestureEvent.modifierKeys;
    uie.device = eInputDevices::TOUCH_PAD;
    uie.phase = UIEvent::Phase::GESTURE;
    uie.physPoint = Vector2(e.trackpadGestureEvent.x, e.trackpadGestureEvent.y);
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
    uie.window = e.window;
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

    if (pressed)
    {
        keyboard.OnKeyPressed(uie.key);
    }
    else
    {
        keyboard.OnKeyUnpressed(uie.key);
    }
    inputSystem->HandleInputEvent(&uie);
}

void Window::HandleKeyChar(const Private::MainDispatcherEvent& e)
{
    UIEvent uie;
    uie.window = e.window;
    uie.keyChar = static_cast<char32_t>(e.keyEvent.key);
    uie.phase = e.keyEvent.isRepeated ? UIEvent::Phase::CHAR_REPEAT : UIEvent::Phase::CHAR;
    uie.device = eInputDevices::KEYBOARD;
    uie.timestamp = e.timestamp / 1000.0;
    uie.modifiers = e.keyEvent.modifierKeys;

    inputSystem->HandleInputEvent(&uie);
}

void Window::SetSurfaceScaleAsync(float32 scale)
{
    if (scale <= 0.0f || scale > 1.0f)
    {
        DVASSERT(false, Format("Window::SetSurfaceScale: specified scale (%f) is out of range (0;1], ignoring", scale).c_str());
        return;
    }

    const float32 currentScale = GetSurfaceScale();
    if (FLOAT_EQUAL(currentScale, scale))
    {
        return;
    }

    windowBackend->SetSurfaceScaleAsync(scale);
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
