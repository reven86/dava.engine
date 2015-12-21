/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Core/Core.h"
#include "Render/Renderer.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UIScreenManager.h"

#include "Platform/SystemTimer.h"
#include "Platform/TemplateWin32/CorePlatformWinUAP.h"
#include "Platform/TemplateWin32/WinUAPXamlApp.h"
#include "Platform/TemplateWin32/DispatcherWinUAP.h"
#include "Platform/DeviceInfo.h"

#include "FileSystem/Logger.h"

#include "Utils/Utils.h"

#include "WinUAPXamlApp.h"
#include "DeferredEvents.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

using namespace ::Windows::System;
using namespace ::Windows::Foundation;
using namespace ::Windows::UI::Core;
using namespace ::Windows::UI::Xaml;
using namespace ::Windows::UI::Xaml::Input;
using namespace ::Windows::UI::Xaml::Controls;
using namespace ::Windows::UI::Input;
using namespace ::Windows::UI::ViewManagement;
using namespace ::Windows::Devices::Input;
using namespace ::Windows::ApplicationModel;
using namespace ::Windows::Graphics::Display;
using namespace ::Windows::ApplicationModel::Core;
using namespace ::Windows::UI::Xaml::Media;
using namespace ::Windows::System::Threading;
using namespace ::Windows::Phone::UI::Input;

namespace DAVA
{
namespace
{
UIEvent::Device ToDavaDeviceId(PointerDeviceType type)
{
    switch (type)
    {
    case PointerDeviceType::Mouse:
        return UIEvent::Device::MOUSE;
    case PointerDeviceType::Pen:
        return UIEvent::Device::PEN;
    case PointerDeviceType::Touch:
        return UIEvent::Device::TOUCH_SURFACE;
    default:
        DVASSERT(false && "can't be!");
        return UIEvent::Device::UNKNOWN;
    }
}
} // anonymous namespace

WinUAPXamlApp::WinUAPXamlApp()
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , isPhoneApiDetected(DeviceInfo::ePlatform::PLATFORM_PHONE_WIN_UAP == DeviceInfo::GetPlatform())
{
    deferredSizeScaleEvents.reset(new DeferredScreenMetricEvents(isPhoneApiDetected, [this](bool isSizeUpdate, float32 widht, float32 height, bool isScaleUpdate, float32 scaleX, float32 scaleY) {
        MetricsScreenUpdated(isSizeUpdate, widht, height, isScaleUpdate, scaleX, scaleY);
    }));
    displayRequest = ref new Windows::System::Display::DisplayRequest;
    AllowDisplaySleep(false);

#if defined(DAVA_WINUAP_MOUSE_HACK)
    if (!isPhoneApiDetected)
    {
        // Here land of black magic and fire-spitting dragons begins
        MEMORY_BASIC_INFORMATION bi;
        VirtualQuery(static_cast<void*>(&GetModuleFileNameA), &bi, sizeof(bi));
        HMODULE hkernel = reinterpret_cast<HMODULE>(bi.AllocationBase);

        HMODULE(WINAPI * LoadLibraryW)
        (LPCWSTR lpLibFileName);
        LoadLibraryW = reinterpret_cast<decltype(LoadLibraryW)>(GetProcAddress(hkernel, "LoadLibraryW"));

        HMODULE huser = LoadLibraryW(L"user32.dll");
        SetCursorPos = reinterpret_cast<decltype(SetCursorPos)>(GetProcAddress(huser, "SetCursorPos"));
    }
#endif
}

WinUAPXamlApp::~WinUAPXamlApp()
{
    AllowDisplaySleep(true);
}

DisplayOrientations WinUAPXamlApp::GetDisplayOrientation()
{
    return displayOrientation;
}

ApplicationViewWindowingMode WinUAPXamlApp::GetScreenMode()
{
    return isFullscreen ? ApplicationViewWindowingMode::FullScreen :
                          ApplicationViewWindowingMode::PreferredLaunchViewSize;
}

void WinUAPXamlApp::SetScreenMode(ApplicationViewWindowingMode screenMode)
{
    // Note: must run on UI thread
    bool fullscreen = ApplicationViewWindowingMode::FullScreen == screenMode;
    SetFullScreen(fullscreen);
}

Windows::Foundation::Size WinUAPXamlApp::GetCurrentScreenSize()
{
    return Windows::Foundation::Size(static_cast<float32>(viewWidth), static_cast<float32>(viewHeight));
}

bool WinUAPXamlApp::SetMouseCaptureMode(InputSystem::eMouseCaptureMode newMode)
{
    // should be started on UI thread
    if (isPhoneApiDetected || !isActivated)
    {
        return false;
    }

    if (mouseCaptureMode != newMode)
    {
        // Setup new capture mode
        switch (newMode)
        {
        case DAVA::InputSystem::eMouseCaptureMode::OFF:
            // Nothing to setup on platform
            mouseCaptureMode = newMode;
            break;
        case DAVA::InputSystem::eMouseCaptureMode::FRAME:
            // Mode unsupported yet
            Logger::Error("Unsupported cursor capture mode");
            break;
        case DAVA::InputSystem::eMouseCaptureMode::PINING:
            mouseCaptureMode = newMode;
            break;
        default:
            DVASSERT("Incorrect cursor capture mode");
            Logger::Error("Incorrect cursor capture mode");
            break;
        }
    }

    return mouseCaptureMode == newMode;
}

bool WinUAPXamlApp::SetCursorVisible(bool isVisible)
{
    // should be started on UI thread
    if (isPhoneApiDetected)
    {
        return isMouseCursorShown == isVisible;
    }

    if (isVisible != isMouseCursorShown)
    {
#if defined(DAVA_WINUAP_MOUSE_HACK)
        if (!isVisible)
        {
            Window::Current->CoreWindow->PointerCursor = nullptr;

            Windows::Foundation::Rect rc = Window::Current->CoreWindow->Bounds;
            float32 centerX = rc.Width / 2.0f;
            float32 centerY = rc.Height / 2.0f;

            float32 scale = DeviceInfo::GetScreenInfo().scale;
            SetCursorPos(static_cast<int>((centerX + rc.X) * scale), static_cast<int>((centerY + rc.Y) * scale));
            skipMouseMoveEvent = true;
        }
        else
        {
            Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
        }
#else
        if (isVisible)
        {
            MouseDevice::GetForCurrentView()->MouseMoved -= token;
            Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
        }
        else
        {
            token = MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice ^, MouseEventArgs ^>(this, &WinUAPXamlApp::OnMouseMoved);
            Window::Current->CoreWindow->PointerCursor = nullptr;
        }
#endif
        isMouseCursorShown = isVisible;
    }
    return true;
}

void WinUAPXamlApp::PreStartAppSettings()
{
    if (isPhoneApiDetected)
    {
        // default orientation landscape and landscape flipped
        // will be changed in SetDisplayOrientations()
        StatusBar::GetForCurrentView()->HideAsync();
    }
    Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->FullScreenSystemOverlayMode = FullScreenSystemOverlayMode::Minimal;
}

void WinUAPXamlApp::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    // If mainLoopThreadStarted is false then app performing cold start
    // else app is restored from background or resumed from suspended state
    if (!mainLoopThreadStarted)
    {
        PreStartAppSettings();
        uiThreadDispatcher = Window::Current->CoreWindow->Dispatcher;

        CreateBaseXamlUI();

        Thread* mainLoopThread = Thread::Create([this, args]() { Run(args); });
        mainLoopThread->Start();
        mainLoopThread->BindToProcessor(0);
        mainLoopThread->SetPriority(Thread::PRIORITY_HIGH);
        mainLoopThread->Release();

        mainLoopThreadStarted = true;
    }
    else
    {
        EmitPushNotification(args);
    }

    Window::Current->Activate();
}

void WinUAPXamlApp::AddUIElement(Windows::UI::Xaml::UIElement^ uiElement)
{
    // Note: must be called from UI thread
    canvas->Children->Append(uiElement);
}

void WinUAPXamlApp::RemoveUIElement(Windows::UI::Xaml::UIElement^ uiElement)
{
    // Note: must be called from UI thread
    unsigned int index = 0;
    for (auto x = canvas->Children->First();x->HasCurrent;x->MoveNext(), ++index)
    {
        if (x->Current == uiElement)
        {
            canvas->Children->RemoveAt(index);
            break;
        }
    }
}

void WinUAPXamlApp::PositionUIElement(Windows::UI::Xaml::UIElement^ uiElement, float32 x, float32 y)
{
    // Note: must be called from UI thread
    canvas->SetLeft(uiElement, x);
    canvas->SetTop(uiElement, y);
}

void WinUAPXamlApp::SetTextBoxCustomStyle(Windows::UI::Xaml::Controls::TextBox^ textBox)
{
    textBox->Style = customTextBoxStyle;
}

void WinUAPXamlApp::SetPasswordBoxCustomStyle(Windows::UI::Xaml::Controls::PasswordBox^ passwordBox)
{
    passwordBox->Style = customPasswordBoxStyle;
}

void WinUAPXamlApp::UnfocusUIElement()
{
    // XAML controls cannot be unfocused programmatically, this is especially useful for text fields
    // So use dummy offscreen control that steals focus
    controlThatTakesFocus->Focus(FocusState::Pointer);
}

void WinUAPXamlApp::Run(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ args)
{
    dispatcher = std::make_unique<DispatcherWinUAP>();
    Core::Instance()->CreateSingletons();
    // View size and orientation option should be configured in FrameworkDidLaunched
    FrameworkDidLaunched();

    core->RunOnUIThreadBlocked([this]() {
        SetupEventHandlers();
        PrepareScreenSize();
        SetTitleName();
        SetDisplayOrientations();
        TrackWindowMinimumSize();

        float32 width = static_cast<float32>(swapChainPanel->ActualWidth);
        float32 height = static_cast<float32>(swapChainPanel->ActualHeight);
        float32 scaleX = swapChainPanel->CompositionScaleX;
        float32 scaleY = swapChainPanel->CompositionScaleY;
        UpdateScreenSizeAndScale(width, height, scaleX, scaleY);
    });

    core->rendererParams.window = reinterpret_cast<void*>(swapChainPanel);
    core->rendererParams.width = viewWidth;
    core->rendererParams.height = viewHeight;
    core->rendererParams.scaleX = viewScaleX;
    core->rendererParams.scaleY = viewScaleY;

    InitCoordinatesSystem();

    Core::Instance()->SetIsActive(true);
    Core::Instance()->SystemAppStarted();

    EmitPushNotification(args);

    SystemTimer* sysTimer = SystemTimer::Instance();
    while (!quitFlag)
    {
        dispatcher->ProcessTasks();

        //  Control FPS
        {
            static uint64 startTime = sysTimer->AbsoluteMS();

            uint64 elapsedTime = sysTimer->AbsoluteMS() - startTime;
            int32 fpsLimit = Renderer::GetDesiredFPS();
            if (fpsLimit > 0)
            {
                uint64 averageFrameTime = 1000UL / static_cast<uint64>(fpsLimit);
                if (averageFrameTime > elapsedTime)
                {
                    uint64 sleepMs = averageFrameTime - elapsedTime;
                    Thread::Sleep(static_cast<uint32>(sleepMs));
                }
            }
            startTime = sysTimer->AbsoluteMS();
        }

        Core::Instance()->SystemProcessFrame();
    }

    ApplicationCore* appCore = Core::Instance()->GetApplicationCore();
    if (appCore != nullptr && appCore->OnQuit())
    {
        Application::Current->Exit();
    }

    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
    Core::Instance()->ReleaseSingletons();

    Application::Current->Exit();
}

void WinUAPXamlApp::OnSuspending(::Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args)
{
    core->RunOnMainThread([]() {
        Core::Instance()->GetApplicationCore()->OnSuspend();
    });
}

void WinUAPXamlApp::OnResuming(::Platform::Object^ sender, ::Platform::Object^ args)
{
    core->RunOnMainThread([]() {
        Core::Instance()->GetApplicationCore()->OnResume();
    });
}

void WinUAPXamlApp::OnWindowActivationChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ args)
{
    CoreWindowActivationState state = args->WindowActivationState;

    core->RunOnMainThread([this, state]() {
        switch (state)
        {
        case CoreWindowActivationState::CodeActivated:
        case CoreWindowActivationState::PointerActivated:
            if (isPhoneApiDetected)
            {
                Core::Instance()->SetIsActive(true);
            }
            else
            {
                Core::Instance()->FocusReceived();
            }
            isActivated = true;
            break;
        case CoreWindowActivationState::Deactivated:
            if (isPhoneApiDetected)
            {
                Core::Instance()->SetIsActive(false);
            }
            else
            {
                Core::Instance()->FocusLost();
            }
            InputSystem::Instance()->GetKeyboard().ClearAllKeys();
            isActivated = false;
            break;
        default:
            break;
        }
    });
}

void WinUAPXamlApp::OnWindowVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args)
{
    bool visible = args->Visible;
    AllowDisplaySleep(!visible);
    core->RunOnMainThread([this, visible]() {
        if (visible)
        {
            if (!isPhoneApiDetected)
            {
                Core::Instance()->GoForeground();
                //Core::Instance()->FocusRecieve();
            }
            Core::Instance()->SetIsActive(true); //TODO: Maybe should move to client side
        }
        else
        {
            if (!isPhoneApiDetected)
            {
                //Core::Instance()->FocusLost();
                Core::Instance()->GoBackground(false);
            }
            else
            {
                Core::Instance()->SetIsActive(false); //TODO: Maybe should move to client side
            }
            InputSystem::Instance()->GetKeyboard().ClearAllKeys();
        }
    });
}

void WinUAPXamlApp::MetricsScreenUpdated(bool isSizeUpdate, float32 width, float32 height, bool isScaleUpdate, float32 scaleX, float32 scaleY)
{
    if (!isSizeUpdate)
    {
        width = static_cast<float32>(swapChainPanel->ActualWidth);
        height = static_cast<float32>(swapChainPanel->ActualHeight);
    }
    if (!isScaleUpdate)
    {
        scaleX = swapChainPanel->CompositionScaleX;
        scaleY = swapChainPanel->CompositionScaleY;
    }
    core->RunOnMainThread([this, width, height, scaleX, scaleY]() {
        UpdateScreenSizeAndScale(width, height, scaleX, scaleY);
        DeviceInfo::InitializeScreenInfo();
        ResetRender();
        ReInitCoordinatesSystem();
        UIScreenManager::Instance()->ScreenSizeChanged();
        UIControlSystem::Instance()->ScreenSizeChanged();
    });
}

WinUAPXamlApp::MouseButtonState WinUAPXamlApp::UpdateMouseButtonsState(Windows::UI::Input::PointerPointProperties ^ pointProperties)
{
    MouseButtonState result;

    if (isLeftButtonPressed != pointProperties->IsLeftButtonPressed)
    {
        result.button = UIEvent::BUTTON_1;
        result.isPressed = pointProperties->IsLeftButtonPressed;
    }

    if (isRightButtonPressed != pointProperties->IsRightButtonPressed)
    {
        result.button = UIEvent::BUTTON_2;
        result.isPressed = pointProperties->IsRightButtonPressed;
    }

    if (isMiddleButtonPressed != pointProperties->IsMiddleButtonPressed)
    {
        result.button = UIEvent::BUTTON_3;
        result.isPressed = pointProperties->IsMiddleButtonPressed;
    }

    isLeftButtonPressed = pointProperties->IsLeftButtonPressed;
    isRightButtonPressed = pointProperties->IsRightButtonPressed;
    isMiddleButtonPressed = pointProperties->IsMiddleButtonPressed;

    return result;
}

void WinUAPXamlApp::OnSwapChainPanelPointerPressed(Platform::Object ^, PointerRoutedEventArgs ^ args)
{
    PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    int32 pointerOrButtonIndex = pointerPoint->PointerId;
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;

    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        MouseButtonState mouseBtnChange = UpdateMouseButtonsState(pointerPoint->Properties);
        pointerOrButtonIndex = mouseBtnChange.button;
    }

    auto fn = [this, x, y, pointerOrButtonIndex, type]() {
        DAVATouchEvent(UIEvent::Phase::BEGAN, x, y, pointerOrButtonIndex, ToDavaDeviceId(type));
    };

    core->RunOnMainThread(fn);
}

void WinUAPXamlApp::OnSwapChainPanelPointerReleased(Platform::Object ^ /*sender*/, PointerRoutedEventArgs ^ args)
{
    PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    int32 pointerOrButtonIndex = pointerPoint->PointerId;
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;

    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        MouseButtonState mouseBtnChange = UpdateMouseButtonsState(pointerPoint->Properties);
        pointerOrButtonIndex = mouseBtnChange.button;
    }

    auto fn = [this, x, y, pointerOrButtonIndex, type]() {
        DAVATouchEvent(UIEvent::Phase::ENDED, x, y, pointerOrButtonIndex, ToDavaDeviceId(type));
    };

    core->RunOnMainThread(fn);
}

void WinUAPXamlApp::OnSwapChainPanelPointerMoved(Platform::Object ^ /*sender*/, PointerRoutedEventArgs ^ args)
{
    if (!isActivated)
    {
        return;
    }

    if (mouseCaptureMode == InputSystem::eMouseCaptureMode::PINING || !isMouseCursorShown)
    {
#if defined(DAVA_WINUAP_MOUSE_HACK)
        PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
        PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;
        if (!isPhoneApiDetected && PointerDeviceType::Mouse == deviceType)
        {
            if (skipMouseMoveEvent)
            {
                skipMouseMoveEvent = false;
                return;
            }

            UIEvent::Phase phase = UIEvent::Phase::MOVE;
            int32 pointerOrButtonIndex = UIEvent::BUTTON_NONE;
            MouseButtonState mouseBtnChange = UpdateMouseButtonsState(pointerPoint->Properties);
            if (UIEvent::BUTTON_NONE != mouseBtnChange.button)
            {
                phase = mouseBtnChange.isPressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
                pointerOrButtonIndex = mouseBtnChange.button;
            }
            else if (isLeftButtonPressed)
            {
                pointerOrButtonIndex = UIEvent::BUTTON_1;
                phase = UIEvent::Phase::DRAG;
            }
            else if (isRightButtonPressed)
            {
                pointerOrButtonIndex = UIEvent::BUTTON_2;
                phase = UIEvent::Phase::DRAG;
            }

            Windows::Foundation::Rect rc = Window::Current->CoreWindow->Bounds;
            float32 centerX = rc.Width / 2.0f;
            float32 centerY = rc.Height / 2.0f;

            float32 x = pointerPoint->Position.X;
            float32 y = pointerPoint->Position.Y;

            float32 deltaX = x - centerX;
            float32 deltaY = y - centerY;

            core->RunOnMainThread([this, deltaX, deltaY, phase, pointerOrButtonIndex]() {
                DAVATouchEvent(phase, deltaX, deltaY, pointerOrButtonIndex, UIEvent::Device::MOUSE);
            });

            float32 scale = DeviceInfo::GetScreenInfo().scale;
            SetCursorPos(static_cast<int>((centerX + rc.X) * scale), static_cast<int>((centerY + rc.Y) * scale));
            skipMouseMoveEvent = true;
        }
#endif
        return;
    }

    UIEvent::Phase phase = UIEvent::Phase::DRAG;
    PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    int32 pointerOrButtonIndex = pointerPoint->PointerId;

    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        MouseButtonState mouseBtnChange = UpdateMouseButtonsState(pointerPoint->Properties);

        if (UIEvent::BUTTON_NONE != mouseBtnChange.button)
        {
            phase = mouseBtnChange.isPressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
            pointerOrButtonIndex = mouseBtnChange.button;
        }
        else if (isLeftButtonPressed)
        {
            pointerOrButtonIndex = UIEvent::BUTTON_1;
        }
        else if (isRightButtonPressed)
        {
            pointerOrButtonIndex = UIEvent::BUTTON_2;
        }
        else
        {
            phase = UIEvent::Phase::MOVE;
        }
    }

    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;

    core->RunOnMainThread([this, phase, x, y, pointerOrButtonIndex, type]() {
        DAVATouchEvent(phase, x, y, pointerOrButtonIndex, ToDavaDeviceId(type));
    });
}

void WinUAPXamlApp::OnSwapChainPanelPointerEntered(Platform::Object ^ /*sender*/, PointerRoutedEventArgs ^ args)
{
    PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if (PointerDeviceType::Mouse == type && mouseCaptureMode == InputSystem::eMouseCaptureMode::PINING)
    {
        SetCursorVisible(false);
    }
}

void WinUAPXamlApp::OnSwapChainPanelPointerExited(Platform::Object ^ /*sender*/, PointerRoutedEventArgs ^ args)
{
    bool passEventForProcession = true;
    PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if (PointerDeviceType::Mouse == type || PointerDeviceType::Pen == type)
    {
        passEventForProcession = isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed;

        PointerPointProperties ^ pointerProperties = pointerPoint->Properties;
        isLeftButtonPressed = pointerProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointerProperties->IsRightButtonPressed;
        isMiddleButtonPressed = pointerProperties->IsMiddleButtonPressed;

        SetCursorVisible(true);
    }

    if (passEventForProcession)
    {
        float32 x = pointerPoint->Position.X;
        float32 y = pointerPoint->Position.Y;
        int32 id = pointerPoint->PointerId;
        core->RunOnMainThread([this, x, y, id, type]() {
            DAVATouchEvent(UIEvent::Phase::ENDED, x, y, id, ToDavaDeviceId(type));
        });
    }
}

void WinUAPXamlApp::OnSwapChainPanelPointerWheel(Platform::Object ^ /*sender*/, PointerRoutedEventArgs ^ args)
{
    PointerPoint ^ pointerPoint = args->GetCurrentPoint(nullptr);
    int32 wheelDelta = pointerPoint->Properties->MouseWheelDelta;
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    Vector2 physPoint(pointerPoint->Position.X, pointerPoint->Position.Y);

    core->RunOnMainThread([this, wheelDelta, physPoint, type]() {
        UIEvent ev;

        ev.scrollDelta.y = wheelDelta / static_cast<float32>(WHEEL_DELTA);
        ev.phase = UIEvent::Phase::WHEEL;
        ev.device = ToDavaDeviceId(type);
        ev.physPoint = physPoint;
        ev.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

        UIControlSystem::Instance()->OnInput(&ev);
    });
}

void WinUAPXamlApp::OnHardwareBackButtonPressed(Platform::Object ^ /*sender*/, BackPressedEventArgs ^ args)
{
    core->RunOnMainThread([this]() {
        UIEvent ev;
        ev.keyChar = 0;
        ev.tapCount = 1;
        ev.phase = UIEvent::Phase::KEY_DOWN;
        ev.tid = DVKEY_BACK;
        ev.device = UIEvent::Device::KEYBOARD;
        ev.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

        UIControlSystem::Instance()->OnInput(&ev);
        InputSystem::Instance()->GetKeyboard().OnKeyPressed(static_cast<uint32>(DVKEY_BACK));

        ev.phase = UIEvent::Phase::KEY_UP;

        UIControlSystem::Instance()->OnInput(&ev);
        InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(static_cast<uint32>(DVKEY_BACK));
    });
    args->Handled = true;
}

void WinUAPXamlApp::OnAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher ^ sender, Windows::UI::Core::AcceleratorKeyEventArgs ^ keyEventArgs)
{
    int32 key = static_cast<uint32>(keyEventArgs->VirtualKey);
    UIEvent::Phase phase;
    switch (keyEventArgs->EventType)
    {
    case CoreAcceleratorKeyEventType::KeyDown:
    case CoreAcceleratorKeyEventType::SystemKeyDown:
        phase = keyEventArgs->KeyStatus.WasKeyDown ? UIEvent::Phase::KEY_DOWN_REPEAT : UIEvent::Phase::KEY_DOWN;
        break;

    case CoreAcceleratorKeyEventType::KeyUp:
    case CoreAcceleratorKeyEventType::SystemKeyUp:
        phase = UIEvent::Phase::KEY_UP;
        break;

    default:
        return;
    }

    core->RunOnMainThread([key, phase]() {
        auto& keyboard = InputSystem::Instance()->GetKeyboard();

        UIEvent uiEvent;
        uiEvent.device = UIEvent::Device::KEYBOARD;
        uiEvent.phase = phase;
        uiEvent.tid = keyboard.GetDavaKeyForSystemKey(key);
        uiEvent.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

        UIControlSystem::Instance()->OnInput(&uiEvent);
        switch (uiEvent.phase)
        {
        case UIEvent::Phase::KEY_DOWN:
        case UIEvent::Phase::KEY_DOWN_REPEAT:
            keyboard.OnSystemKeyPressed(key);
            break;

        case UIEvent::Phase::KEY_UP:
            keyboard.OnSystemKeyUnpressed(key);
            break;
        }
    });
}

void WinUAPXamlApp::OnChar(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::CharacterReceivedEventArgs ^ args)
{
    uint32 unicodeChar = args->KeyCode;
    bool isRepeat = args->KeyStatus.WasKeyDown;
    core->RunOnMainThread([this, unicodeChar, isRepeat]() {
        UIEvent ev;
        DVASSERT(unicodeChar < 0xFFFF); // wchar_t is 16 bit, so keyChar dosnt fit
        ev.keyChar = unicodeChar;
        ev.device = UIEvent::Device::KEYBOARD;
        ev.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

        if (isRepeat)
        {
            ev.phase = UIEvent::Phase::CHAR_REPEAT;
        }
        else
        {
            ev.phase = UIEvent::Phase::CHAR;
        }
        UIControlSystem::Instance()->OnInput(&ev);
    });
}

void WinUAPXamlApp::OnMouseMoved(MouseDevice^ mouseDevice, MouseEventArgs^ args)
{
    if (mouseCaptureMode != InputSystem::eMouseCaptureMode::PINING || isMouseCursorShown)
    {
        return;
    }

    float32 x = static_cast<float32>(args->MouseDelta.X);
    float32 y = static_cast<float32>(args->MouseDelta.Y);

    UIEvent::Phase phase = UIEvent::Phase::MOVE;
    int32 pointerOrButtonIndex = UIEvent::BUTTON_NONE;
    
    PointerPoint ^ pointerPoint = nullptr;
    try
    {
        pointerPoint = Windows::UI::Input::PointerPoint::GetCurrentPoint(1);
    }
    catch (Platform::Exception^ e)
    {
        Logger::FrameworkDebug("Exception in WinUAPXamlApp::OnMouseMoved: 0x%08X - %s", e->HResult, RTStringToString(e->Message).c_str());
    }
    if (nullptr != pointerPoint)
    {
        MouseButtonState mouseBtnChange = UpdateMouseButtonsState(pointerPoint->Properties);
        if (UIEvent::BUTTON_NONE != mouseBtnChange.button)
        {
            phase = mouseBtnChange.isPressed ? UIEvent::Phase::BEGAN : UIEvent::Phase::ENDED;
            pointerOrButtonIndex = mouseBtnChange.button;
        }
        else if (isLeftButtonPressed)
        {
            pointerOrButtonIndex = UIEvent::BUTTON_1;
            phase = UIEvent::Phase::DRAG;
        }
        else if (isRightButtonPressed)
        {
            pointerOrButtonIndex = UIEvent::BUTTON_2;
            phase = UIEvent::Phase::DRAG;
        }
    }

    core->RunOnMainThread([this, x, y, phase, pointerOrButtonIndex]() {
        DAVATouchEvent(phase, x, y, pointerOrButtonIndex, UIEvent::Device::MOUSE);
    });
}

void WinUAPXamlApp::DAVATouchEvent(UIEvent::Phase phase, float32 x, float32 y, int32 id, UIEvent::Device device)
{
    UIEvent newTouch;

    newTouch.tid = id;
    newTouch.physPoint.x = x;
    newTouch.physPoint.y = y;
    newTouch.point.x = x;
    newTouch.point.y = y;
    newTouch.phase = phase;
    newTouch.device = device;
    newTouch.tapCount = 1;
    newTouch.timestamp = (SystemTimer::FrameStampTimeMS() / 1000.0);

    UIControlSystem::Instance()->OnInput(&newTouch);
}

void WinUAPXamlApp::SetupEventHandlers()
{
    Suspending += ref new SuspendingEventHandler(this, &WinUAPXamlApp::OnSuspending);
    Resuming += ref new EventHandler<::Platform::Object^>(this, &WinUAPXamlApp::OnResuming);

    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    coreWindow->Activated += ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &WinUAPXamlApp::OnWindowActivationChanged);
    coreWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinUAPXamlApp::OnWindowVisibilityChanged);

    auto coreWindowSizeChanged = ref new TypedEventHandler<CoreWindow ^, WindowSizeChangedEventArgs ^>([this](CoreWindow ^ coreWindow, WindowSizeChangedEventArgs ^ arg) {
        deferredSizeScaleEvents->CoreWindowSizeChanged(coreWindow, arg);
    });
    coreWindow->SizeChanged += coreWindowSizeChanged;

    auto swapChainPanelSizeChanged = ref new SizeChangedEventHandler([this](Object ^ sender, SizeChangedEventArgs ^ e) {
        deferredSizeScaleEvents->SwapChainPanelSizeChanged(sender, e);
    });
    auto swapChainCompositionScaleChanged = ref new TypedEventHandler<SwapChainPanel ^, Object ^>([this](SwapChainPanel ^ panel, Object ^ args) {
        deferredSizeScaleEvents->SwapChainPanelCompositionScaleChanged(panel, args);
    });
    swapChainPanel->SizeChanged += swapChainPanelSizeChanged;
    swapChainPanel->CompositionScaleChanged += swapChainCompositionScaleChanged;

    // Receive mouse events from SwapChainPanel, not CoreWindow, to not handle native controls' events
    swapChainPanel->PointerPressed += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerPressed);
    swapChainPanel->PointerReleased += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerReleased);
    swapChainPanel->PointerEntered += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerEntered);
    swapChainPanel->PointerMoved += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerMoved);
    swapChainPanel->PointerExited += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerExited);
    swapChainPanel->PointerWheelChanged += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerWheel);

    coreWindow->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher ^, AcceleratorKeyEventArgs ^>(this, &WinUAPXamlApp::OnAcceleratorKeyActivated);
    coreWindow->CharacterReceived += ref new TypedEventHandler<CoreWindow ^, CharacterReceivedEventArgs ^>(this, &WinUAPXamlApp::OnChar);

    if (isPhoneApiDetected)
    {
        HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &WinUAPXamlApp::OnHardwareBackButtonPressed);
    }
}

void WinUAPXamlApp::CreateBaseXamlUI()
{
    swapChainPanel = ref new Controls::SwapChainPanel();
    canvas = ref new Controls::Canvas();
    swapChainPanel->Children->Append(canvas);
    Window::Current->Content = swapChainPanel;

    // Windows UAP doesn't allow to unfocus UI control programmatically
    // It only permits to set focus at another control
    // So create dummy offscreen button that steals focus when there is
    // a need to unfocus native control, especially useful for text fields
    controlThatTakesFocus = ref new Button();
    controlThatTakesFocus->Content = L"I steal your focus";
    controlThatTakesFocus->Width = 30;
    controlThatTakesFocus->Height = 20;
    AddUIElement(controlThatTakesFocus);
    PositionUIElement(controlThatTakesFocus, -100, -100);

    {   // Load custom textbox and password styles that allow transparent background when control has focus
        using Windows::UI::Xaml::Markup::XamlReader;

        Platform::Object^ obj = XamlReader::Load(ref new Platform::String(xamlTextBoxStyles));
        ResourceDictionary^ dict = (ResourceDictionary^)obj;

        Resources->MergedDictionaries->Append(dict);
        Object^ texboxStyleObj = Resources->Lookup(ref new Platform::String(L"dava_custom_textbox"));
        Object^ passwordStyleObj = Resources->Lookup(ref new Platform::String(L"dava_custom_passwordbox"));

        customTextBoxStyle = (Windows::UI::Xaml::Style^)texboxStyleObj;
        customPasswordBoxStyle = (Windows::UI::Xaml::Style^)passwordStyleObj;
    }
}

void WinUAPXamlApp::SetTitleName()
{
    // Note: must run on UI thread
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (nullptr != options)
    {
        WideString title = StringToWString(options->GetString("title", "[set application title using core options property 'title']"));
        ApplicationView::GetForCurrentView()->Title = ref new ::Platform::String(title.c_str());
    }
}

void WinUAPXamlApp::SetDisplayOrientations()
{
    // Note: must run on UI thread
    Core::eScreenOrientation orientMode = Core::Instance()->GetScreenOrientation();
    switch (orientMode)
    {
    case Core::SCREEN_ORIENTATION_TEXTURE:
        break;
    case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        displayOrientation = DisplayOrientations::Landscape;
        break;
    case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        displayOrientation = DisplayOrientations::LandscapeFlipped;
        break;
    case Core::SCREEN_ORIENTATION_PORTRAIT:
        displayOrientation = DisplayOrientations::Portrait;
        break;
    case Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
        displayOrientation = DisplayOrientations::PortraitFlipped;
        break;
    case Core::SCREEN_ORIENTATION_LANDSCAPE_AUTOROTATE:
        displayOrientation = DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;
        break;
    case Core::SCREEN_ORIENTATION_PORTRAIT_AUTOROTATE:
        displayOrientation = DisplayOrientations::Portrait | DisplayOrientations::PortraitFlipped;
        break;
    }
    DisplayInformation::GetForCurrentView()->AutoRotationPreferences = displayOrientation;
}

void WinUAPXamlApp::TrackWindowMinimumSize()
{
    if (!isPhoneApiDetected)
    {
        const KeyedArchive* options = core->GetOptions();
        int32 minWidth = options->GetInt32("min-width", 0);
        int32 minHeight = options->GetInt32("min-height", 0);
        if (minWidth > 0 && minHeight > 0)
        {
            // Note: the largest allowed minimum size is 500 x 500 effective pixels
            // https://msdn.microsoft.com/en-us/library/windows/apps/windows.ui.viewmanagement.applicationview.setpreferredminsize.aspx
            Size size(static_cast<float32>(minWidth), static_cast<float32>(minHeight));
            ApplicationView::GetForCurrentView()->SetPreferredMinSize(size);

            deferredSizeScaleEvents->TrackWindowMinimumSize(minWidth, minHeight);
        }
    }
}

void WinUAPXamlApp::ResetScreen()
{
    core->RunOnUIThreadBlocked([this]() {
        float32 width = static_cast<float32>(swapChainPanel->ActualWidth);
        float32 height = static_cast<float32>(swapChainPanel->ActualHeight);
        float32 scaleX = swapChainPanel->CompositionScaleX;
        float32 scaleY = swapChainPanel->CompositionScaleY;
        UpdateScreenSizeAndScale(width, height, scaleX, scaleY);
    });

    ResetRender();
    ReInitCoordinatesSystem();
    UIScreenManager::Instance()->ScreenSizeChanged();
}

void WinUAPXamlApp::ResetRender()
{
    rhi::ResetParam params;
    params.width = viewWidth;
    params.height = viewHeight;
    params.scaleX = viewScaleX;
    params.scaleY = viewScaleY;
    Renderer::Reset(params);
}

void WinUAPXamlApp::InitCoordinatesSystem()
{
    VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
    virtSystem->SetInputScreenAreaSize(viewWidth, viewHeight); //TODO: move to FrameworkMain
    virtSystem->SetPhysicalScreenSize(physicalWidth, physicalHeight); //TODO: move to FrameworkMain
    virtSystem->EnableReloadResourceOnResize(true);
}

void WinUAPXamlApp::ReInitCoordinatesSystem()
{
    VirtualCoordinatesSystem* virtSystem = VirtualCoordinatesSystem::Instance();
    virtSystem->SetInputScreenAreaSize(viewWidth, viewHeight);
    virtSystem->SetPhysicalScreenSize(physicalWidth, physicalHeight);
    virtSystem->ScreenSizeChanged();
}

void WinUAPXamlApp::PrepareScreenSize()
{
    // Note: must run on UI thread
    bool isFull(false);
    KeyedArchive* options = Core::Instance()->GetOptions();
    if (nullptr != options)
    {
        windowedMode.width = options->GetInt32("width", DisplayMode::DEFAULT_WIDTH);
        windowedMode.height = options->GetInt32("height", DisplayMode::DEFAULT_HEIGHT);
        windowedMode.bpp = options->GetInt32("bpp", DisplayMode::DEFAULT_BITS_PER_PIXEL);
        isFull = (0 != options->GetInt32("fullscreen", 0));
    }
    SetFullScreen(isFull);
    if (!isFullscreen)
    {
        // in units of effective (view) pixels
        SetPreferredSize(static_cast<float32>(windowedMode.width), static_cast<float32>(windowedMode.height));
    }
}

void WinUAPXamlApp::UpdateScreenSizeAndScale(float32 width, float32 height, float32 scaleX, float32 scaleY)
{
    float32 userScale = Core::Instance()->GetScreenScaleMultiplier();
    viewScaleX = scaleX * userScale;
    viewScaleY = scaleY * userScale;
    viewWidth = static_cast<int32>(width);
    viewHeight = static_cast<int32>(height);
    physicalWidth = static_cast<int32>(width * viewScaleX);
    physicalHeight = static_cast<int32>(height * viewScaleY);
}

void WinUAPXamlApp::SetFullScreen(bool isFullscreen_)
{
    // Note: must run on UI thread
    ApplicationView^ view = ApplicationView::GetForCurrentView();
    if (view->IsFullScreenMode == isFullscreen_)
    {
        isFullscreen = isFullscreen_;
        return;
    }
    if (isPhoneApiDetected)
    {
        return;
    }
    if (isFullscreen_)
    {
        isFullscreen = view->TryEnterFullScreenMode();
    }
    else
    {
        view->ExitFullScreenMode();
        isFullscreen = false;
    }
}

void WinUAPXamlApp::SetPreferredSize(float32 width, float32 height)
{
    // Note: must run on UI thread
    if (isPhoneApiDetected)
    {
        return;
    }
    // MSDN::This property only has an effect when the app is launched on a desktop device that is not in tablet mode.
    ApplicationView::GetForCurrentView()->PreferredLaunchViewSize = Windows::Foundation::Size(width, height);
    ApplicationView::PreferredLaunchWindowingMode = ApplicationViewWindowingMode::PreferredLaunchViewSize;
}

void WinUAPXamlApp::EmitPushNotification(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ args)
{
    DVASSERT(nullptr != dispatcher);
    dispatcher->RunAsync([=]() {
        pushNotificationSignal.Emit(args);
    });
}

void WinUAPXamlApp::AllowDisplaySleep(bool sleep)
{
    if (sleep)
    {
        displayRequest->RequestRelease();
    }
    else
    {
        displayRequest->RequestActive();
    }
}

const wchar_t* WinUAPXamlApp::xamlTextBoxStyles = LR"(
<ResourceDictionary
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
    xmlns:local="using:App2">
    <Style x:Key="dava_custom_textbox" TargetType="TextBox">
        <Setter Property="MinWidth" Value="0" />
        <Setter Property="MinHeight" Value="0" />
        <Setter Property="Foreground" Value="White" />
        <Setter Property="Background" Value="Transparent" />
        <Setter Property="BorderBrush" Value="Transparent" />
        <Setter Property="SelectionHighlightColor" Value="{ThemeResource TextSelectionHighlightColorThemeBrush}" />
        <Setter Property="BorderThickness" Value="0" />
        <Setter Property="FontFamily" Value="{ThemeResource ContentControlThemeFontFamily}" />
        <Setter Property="FontSize" Value="{ThemeResource ControlContentThemeFontSize}" />
        <Setter Property="ScrollViewer.HorizontalScrollBarVisibility" Value="Hidden" />
        <Setter Property="ScrollViewer.VerticalScrollBarVisibility" Value="Hidden" />
        <Setter Property="ScrollViewer.IsDeferredScrollingEnabled" Value="False" />
        <Setter Property="Padding" Value="0"/>
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TextBox">
                    <Grid>
                        <ContentPresenter x:Name="HeaderContentPresenter"
                                      Grid.Row="0"
                                      Margin="0,4,0,4"
                                      Grid.ColumnSpan="2"
                                      Content="{TemplateBinding Header}"
                                      ContentTemplate="{TemplateBinding HeaderTemplate}"
                                      FontWeight="Semilight" />
                        <ScrollViewer x:Name="ContentElement"
                                    Grid.Row="1"
                                    HorizontalScrollMode="{TemplateBinding ScrollViewer.HorizontalScrollMode}"
                                    HorizontalScrollBarVisibility="{TemplateBinding ScrollViewer.HorizontalScrollBarVisibility}"
                                    VerticalScrollMode="{TemplateBinding ScrollViewer.VerticalScrollMode}"
                                    VerticalScrollBarVisibility="{TemplateBinding ScrollViewer.VerticalScrollBarVisibility}"
                                    IsHorizontalRailEnabled="{TemplateBinding ScrollViewer.IsHorizontalRailEnabled}"
                                    IsVerticalRailEnabled="{TemplateBinding ScrollViewer.IsVerticalRailEnabled}"
                                    IsDeferredScrollingEnabled="{TemplateBinding ScrollViewer.IsDeferredScrollingEnabled}"
                                    Margin="{TemplateBinding BorderThickness}"
                                    Padding="{TemplateBinding Padding}"
                                    IsTabStop="False"
                                    AutomationProperties.AccessibilityView="Raw"
                                    ZoomMode="Disabled" />
                    </Grid>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style x:Key="dava_custom_passwordbox" TargetType="PasswordBox">
        <Setter Property="MinWidth" Value="0" />
        <Setter Property="MinHeight" Value="0" />
        <Setter Property="Foreground" Value="White" />
        <Setter Property="Background" Value="Transparent" />
        <Setter Property="SelectionHighlightColor" Value="{ThemeResource TextSelectionHighlightColorThemeBrush}" />
        <Setter Property="BorderBrush" Value="Transparent" />
        <Setter Property="BorderThickness" Value="0" />
        <Setter Property="FontFamily" Value="{ThemeResource ContentControlThemeFontFamily}" />
        <Setter Property="FontSize" Value="{ThemeResource ControlContentThemeFontSize}" />
        <Setter Property="ScrollViewer.HorizontalScrollBarVisibility" Value="Hidden" />
        <Setter Property="ScrollViewer.VerticalScrollBarVisibility" Value="Hidden" />
        <Setter Property="Padding" Value="0"/>
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="PasswordBox">
                    <Grid>
                        <ContentPresenter x:Name="HeaderContentPresenter"
                                      Grid.Row="0"
                                      Margin="0,4,0,4"
                                      Grid.ColumnSpan="2"
                                      Content="{TemplateBinding Header}"
                                      ContentTemplate="{TemplateBinding HeaderTemplate}"
                                      FontWeight="Semilight" />
                        <ScrollViewer x:Name="ContentElement"
                            Grid.Row="1"
                                  HorizontalScrollMode="{TemplateBinding ScrollViewer.HorizontalScrollMode}"
                                  HorizontalScrollBarVisibility="{TemplateBinding ScrollViewer.HorizontalScrollBarVisibility}"
                                  VerticalScrollMode="{TemplateBinding ScrollViewer.VerticalScrollMode}"
                                  VerticalScrollBarVisibility="{TemplateBinding ScrollViewer.VerticalScrollBarVisibility}"
                                  IsHorizontalRailEnabled="{TemplateBinding ScrollViewer.IsHorizontalRailEnabled}"
                                  IsVerticalRailEnabled="{TemplateBinding ScrollViewer.IsVerticalRailEnabled}"
                                  Margin="{TemplateBinding BorderThickness}"
                                  Padding="{TemplateBinding Padding}"
                                  IsTabStop="False"
                                  ZoomMode="Disabled"
                                  AutomationProperties.AccessibilityView="Raw"/>
                    </Grid>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
</ResourceDictionary>
)";

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
