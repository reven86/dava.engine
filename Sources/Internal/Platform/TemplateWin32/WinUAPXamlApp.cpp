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
using namespace ::Platform;

namespace DAVA
{

WinUAPXamlApp::WinUAPXamlApp()
    : core(static_cast<CorePlatformWinUAP*>(Core::Instance()))
    , isPhoneApiDetected(DeviceInfo::ePlatform::PLATFORM_PHONE_WIN_UAP == DeviceInfo::GetPlatform())
{
    deferredSizeScaleEvents = new DeferredScreenMetricEvents(DEFERRED_INTERVAL_MSEC, [this](bool isSizeUpdate, float32 widht, float32 height, bool isScaleUpdate, float32 scaleX, float32 scaleY) { MetricsScreenUpdated(isSizeUpdate, widht, height, isScaleUpdate, scaleX, scaleY); });
}

WinUAPXamlApp::~WinUAPXamlApp() 
{
    delete deferredSizeScaleEvents;
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

void WinUAPXamlApp::ToggleFullscreen()
{
    SetFullScreen(!isFullscreen);
}

Windows::Foundation::Size WinUAPXamlApp::GetCurrentScreenSize()
{
    return Windows::Foundation::Size(static_cast<float32>(viewWidth), static_cast<float32>(viewHeight));
}

void WinUAPXamlApp::SetCursorPinning(bool isPinning)
{
    // should be started on UI thread
    if (isPhoneApiDetected)
    {
        return;
    }
    isCursorPinning = isPinning;
}

void WinUAPXamlApp::SetCursorVisible(bool isVisible)
{
    // should be started on UI thread
    if (isPhoneApiDetected)
    {
        return;
    }
    if (isVisible != isMouseCursorShown)
    {
        Window::Current->CoreWindow->PointerCursor = (isVisible ? ref new CoreCursor(CoreCursorType::Arrow, 0) : nullptr);
        isMouseCursorShown = isVisible;
    }
}

void WinUAPXamlApp::PreStartAppSettings()
{
    DisplayInformation::GetForCurrentView()->AutoRotationPreferences = DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;
    if (isPhoneApiDetected)
    {
        // default orientation landscape and landscape flipped
        // will be changed in SetDisplayOrientations()
        StatusBar::GetForCurrentView()->HideAsync();
        Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->SuppressSystemOverlays = true;
    }
}

void WinUAPXamlApp::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    PreStartAppSettings();
    uiThreadDispatcher = Window::Current->CoreWindow->Dispatcher;

    CreateBaseXamlUI();

    WorkItemHandler^ workItemHandler = ref new WorkItemHandler([this](Windows::Foundation::IAsyncAction^ action) { Run(); });
    renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

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

void WinUAPXamlApp::NativeControlGotFocus(Control^ control)
{
    currentFocusedControl = control;
}

void WinUAPXamlApp::NativeControlLostFocus(Control^ control)
{
    if (currentFocusedControl == control)
    {
        currentFocusedControl = nullptr;
    }
}

void WinUAPXamlApp::Run()
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
        
        UpdateScreenSize(static_cast<float32>(swapChainPanel->ActualWidth), static_cast<float32>(swapChainPanel->ActualHeight));
        UpdateScreenScale(swapChainPanel->CompositionScaleX, swapChainPanel->CompositionScaleY);
    });

    core->rendererParams.window = reinterpret_cast<void*>(swapChainPanel);
    core->rendererParams.width = viewWidth;
    core->rendererParams.height = viewHeight;
    core->rendererParams.scaleX = viewScaleX;
    core->rendererParams.scaleY = viewScaleY;

    InitCoordinatesSystem();

    Core::Instance()->SetIsActive(true);

    Core::Instance()->SystemAppStarted();
    while (!quitFlag)
    {
        dispatcher->ProcessTasks();

        uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
        
        Core::Instance()->SystemProcessFrame();
        
        uint32 elapsedTime = (uint32)(SystemTimer::Instance()->AbsoluteMS() - startTime);
        int32 sleepMs = 1;
        int32 fps = Renderer::GetDesiredFPS();
        if (fps > 0)
        {
            sleepMs = (1000 / fps) - elapsedTime;
            if (sleepMs > 0)
            {
                Thread::Sleep(sleepMs);
            }
        }
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
    isWindowVisible = false;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinUAPXamlApp::OnResuming(::Platform::Object^ sender, ::Platform::Object^ args)
{
    isWindowVisible = true;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinUAPXamlApp::OnWindowActivationChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ args)
{
    CoreWindowActivationState state = args->WindowActivationState;
    switch (state)
    {
    case CoreWindowActivationState::CodeActivated:
    case CoreWindowActivationState::PointerActivated:
        Core::Instance()->SetIsActive(true);
        break;
    case CoreWindowActivationState::Deactivated:
        Core::Instance()->SetIsActive(false);
        break;
    default:
        break;
    }
}

void WinUAPXamlApp::OnWindowVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args)
{
    isWindowVisible = args->Visible;
    Core::Instance()->SetIsActive(isWindowVisible);
}

void WinUAPXamlApp::MetricsScreenUpdated(bool isSizeUpdate, float32 widht, float32 height, bool isScaleUpdate, float32 scaleX, float32 scaleY)
{
    if (!isSizeUpdate)
    {
        widht = static_cast<float32>(swapChainPanel->ActualWidth);
        height = static_cast<float32>(swapChainPanel->ActualHeight);
    }
    if (!isScaleUpdate)
    {
        scaleX = swapChainPanel->CompositionScaleX;
        scaleY = swapChainPanel->CompositionScaleY;
    }
    core->RunOnMainThread([this, widht, height, scaleX, scaleY]() {
        UpdateScreenSize(widht, height);
        UpdateScreenScale(scaleX, scaleY);
        ResetRender();
        ReInitCoordinatesSystem();
        UIScreenManager::Instance()->ScreenSizeChanged();
    });
}

void WinUAPXamlApp::OnSwapChainPanelPointerPressed(Platform::Object^ /*sender*/, PointerRoutedEventArgs^ args)
{
    PointerPoint^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        PointerPointProperties^ pointerProperties = pointerPoint->Properties;
        isLeftButtonPressed = pointerProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointerProperties->IsRightButtonPressed;
        isMiddleButtonPressed = pointerProperties->IsMiddleButtonPressed;
    }

    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    int32 id = pointerPoint->PointerId;
    core->RunOnMainThread([this, x, y, id]()
    {
        DAVATouchEvent(UIEvent::PHASE_BEGAN, x, y, id);
    });
}

void WinUAPXamlApp::OnSwapChainPanelPointerReleased(Platform::Object^ /*sender*/, PointerRoutedEventArgs^ args)
{
    bool passEventForProcession = true;
    PointerPoint^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        passEventForProcession = isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed;

        PointerPointProperties^ pointerProperties = pointerPoint->Properties;
        isLeftButtonPressed = pointerProperties->IsLeftButtonPressed;
        isRightButtonPressed = pointerProperties->IsRightButtonPressed;
        isMiddleButtonPressed = pointerProperties->IsMiddleButtonPressed;
    }

    if (passEventForProcession)
    {
        float32 x = pointerPoint->Position.X;
        float32 y = pointerPoint->Position.Y;
        int32 id = pointerPoint->PointerId;
        core->RunOnMainThread([this, x, y, id]() {
            DAVATouchEvent(UIEvent::PHASE_ENDED, x, y, id);
        });
    }
}

void WinUAPXamlApp::OnSwapChainPanelPointerMoved(Platform::Object^ /*sender*/, PointerRoutedEventArgs^ args)
{
    UIEvent::eInputPhase phase = UIEvent::PHASE_DRAG;
    PointerPoint^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if ((PointerDeviceType::Mouse == type) || (PointerDeviceType::Pen == type))
    {
        if (!(isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed))
        {
            phase = UIEvent::PHASE_MOVE;
        }
    }

    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    int32 id = pointerPoint->PointerId;
    core->RunOnMainThread([this, phase, x, y, id]() {
        DAVATouchEvent(phase, x, y, id);
    });
}

void WinUAPXamlApp::OnSwapChainPanelPointerEntered(Platform::Object^ /*sender*/, PointerRoutedEventArgs^ args)
{
    PointerPoint^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if (PointerDeviceType::Mouse == type && isCursorPinning)
    {
        SetCursorVisible(false);
    }
}

void WinUAPXamlApp::OnSwapChainPanelPointerExited(Platform::Object^ /*sender*/, PointerRoutedEventArgs^ args)
{
    bool passEventForProcession = true;
    PointerPoint^ pointerPoint = args->GetCurrentPoint(nullptr);
    PointerDeviceType type = pointerPoint->PointerDevice->PointerDeviceType;
    if (PointerDeviceType::Mouse == type || PointerDeviceType::Pen == type)
    {
        passEventForProcession = isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed;

        PointerPointProperties^ pointerProperties = pointerPoint->Properties;
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
        core->RunOnMainThread([this, x, y, id]() {
            DAVATouchEvent(UIEvent::PHASE_ENDED, x, y, id);
        });
    }
}

void WinUAPXamlApp::OnSwapChainPanelPointerWheel(Platform::Object^ /*sender*/, PointerRoutedEventArgs^ args)
{
    PointerPoint^ pointerPoint = args->GetCurrentPoint(nullptr);
    int32 wheelDelta = pointerPoint->Properties->MouseWheelDelta;
    core->RunOnMainThread([this, wheelDelta]()
    {
        UIEvent newTouch;
        newTouch.tid = 0;
        newTouch.physPoint.x = 0;
        newTouch.physPoint.y = static_cast<float32>(wheelDelta / WHEEL_DELTA);
        newTouch.phase = UIEvent::PHASE_WHEEL;

        UIControlSystem::Instance()->OnInput(UIEvent::PHASE_WHEEL, { newTouch }, allTouches);
    });
}

void WinUAPXamlApp::OnHardwareBackButtonPressed(Platform::Object^ /*sender*/, BackPressedEventArgs ^args)
{
    core->RunOnMainThread([this]()
    {
        UIEvent ev;
        ev.keyChar = 0;
        ev.tapCount = 1;
        ev.phase = UIEvent::PHASE_KEYCHAR;
        ev.tid = DVKEY_BACK;

        Vector<UIEvent> touches = { ev };
        InputSystem::Instance()->GetKeyboard().OnKeyPressed(static_cast<int32>(DVKEY_BACK));
        UIControlSystem::Instance()->OnInput(0, touches, allTouches);
        touches.pop_back();
        UIControlSystem::Instance()->OnInput(0, touches, allTouches);
        InputSystem::Instance()->GetKeyboard().OnKeyUnpressed(static_cast<int32>(DVKEY_BACK));
    });
    args->Handled = true;
}

void WinUAPXamlApp::OnKeyDown(CoreWindow^ /*sender*/, KeyEventArgs^ args)
{
    // Check whether native control has focus, if so ignore key events
    // Explanation:
    //   For now key event handlers are invoked both for focused native control (e.g. TextBox) and for WinUAPXamlApp
    //   This check prevents handling key events considered for native control but native control must call 
    //   NativeControlGotFocus() and NativeControlLostFocus() methods on getting and losing its focus respectively
    if (currentFocusedControl != nullptr && currentFocusedControl->FocusState != FocusState::Unfocused)
        return;

    CoreWindow^ window = CoreWindow::GetForCurrentThread();
    CoreVirtualKeyStates menuStatus = window->GetKeyState(VirtualKey::Menu);
    CoreVirtualKeyStates tabStatus = window->GetKeyState(VirtualKey::Tab);
    bool isPressOrLock = static_cast<bool>((menuStatus & CoreVirtualKeyStates::Down) & (tabStatus & CoreVirtualKeyStates::Down));
    if (isPressOrLock)
    {
        __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__
    }

    int32 key = static_cast<int32>(args->VirtualKey);
    core->RunOnMainThread([this, key]()
    {
        UIEvent ev;
        ev.keyChar = 0;
        ev.tapCount = 1;
        ev.phase = UIEvent::PHASE_KEYCHAR;
        ev.tid = InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(key);

        Vector<UIEvent> touches = { ev };
        UIControlSystem::Instance()->OnInput(0, touches, allTouches);
        touches.pop_back();
        UIControlSystem::Instance()->OnInput(0, touches, allTouches);
        InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(key);
    });
}

void WinUAPXamlApp::OnKeyUp(CoreWindow^ /*sender*/, KeyEventArgs^ args)
{
    // See comment for OnKeyDown
    if (currentFocusedControl != nullptr && currentFocusedControl->FocusState != FocusState::Unfocused)
        return;

    int32 key = static_cast<int32>(args->VirtualKey);
    core->RunOnMainThread([this, key]()
    {
        InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(key);
    });
}

void WinUAPXamlApp::OnMouseMoved(MouseDevice^ mouseDevice, MouseEventArgs^ args)
{
    if (!isCursorPinning || isMouseCursorShown)
    {
        return;
    }

    float32 x = static_cast<float32>(args->MouseDelta.X);
    float32 y = static_cast<float32>(args->MouseDelta.Y);
    int32 id = isLeftButtonPressed + isRightButtonPressed + isMiddleButtonPressed;

    bool mouseButtonPressed = isLeftButtonPressed || isMiddleButtonPressed || isRightButtonPressed;
    UIEvent::eInputPhase phase = mouseButtonPressed ? UIEvent::PHASE_DRAG : UIEvent::PHASE_MOVE;
    core->RunOnMainThread([this, phase, x, y, id]()
    {
        DAVATouchEvent(phase, x, y, id);
    });
}

void WinUAPXamlApp::DAVATouchEvent(UIEvent::eInputPhase phase, float32 x, float32 y, int32 id)
{
    Vector<UIEvent> touches;
    bool isFind = false;
    for (auto it = allTouches.begin(), end = allTouches.end(); it != end; ++it)
    {
        if (it->tid == id)
        {
            isFind = true;
            it->physPoint.x = x;
            it->physPoint.y = y;
            it->phase = phase;
            break;
        }
    }
    if (!isFind)
    {
        UIEvent newTouch;
        newTouch.tid = id;
        newTouch.physPoint.x = x;
        newTouch.physPoint.y = y;
        newTouch.phase = phase;
        allTouches.push_back(newTouch);
    }
    for (auto it = allTouches.begin(), end = allTouches.end(); it != end; ++it)
    {
        touches.push_back(*it);
    }
    if (phase == UIEvent::PHASE_ENDED)
    {
        for (Vector<UIEvent>::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
        {
            if (it->tid == id)
            {
                allTouches.erase(it);
                break;
            }
        }
    }
    UIControlSystem::Instance()->OnInput(phase, touches, allTouches);
}

void WinUAPXamlApp::SetupEventHandlers()
{
    Suspending += ref new SuspendingEventHandler(this, &WinUAPXamlApp::OnSuspending);
    Resuming += ref new EventHandler<::Platform::Object^>(this, &WinUAPXamlApp::OnResuming);

    CoreWindow^ coreWindow = Window::Current->CoreWindow;
    coreWindow->Activated += ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &WinUAPXamlApp::OnWindowActivationChanged);
    coreWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinUAPXamlApp::OnWindowVisibilityChanged);

    auto slowSize = ref new SizeChangedEventHandler([this](Object^ sender, SizeChangedEventArgs^ e) { deferredSizeScaleEvents->UpdateSize(sender, e); });
    auto slowScale = ref new TypedEventHandler<SwapChainPanel^, Object^>([this](SwapChainPanel^ panel, Object^ args) { deferredSizeScaleEvents->UpdateScale(panel, args); });
    swapChainPanel->SizeChanged += slowSize;
    swapChainPanel->CompositionScaleChanged += slowScale;

    // Receive mouse events from SwapChainPanel, not CoreWindow, to not handle native controls' events
    swapChainPanel->PointerPressed += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerPressed);
    swapChainPanel->PointerReleased += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerReleased);
    swapChainPanel->PointerMoved += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerMoved);
    swapChainPanel->PointerEntered += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerEntered);
    swapChainPanel->PointerExited += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerExited);
    swapChainPanel->PointerWheelChanged += ref new PointerEventHandler(this, &WinUAPXamlApp::OnSwapChainPanelPointerWheel);

    MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &WinUAPXamlApp::OnMouseMoved);

    coreWindow->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUAPXamlApp::OnKeyDown);
    coreWindow->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUAPXamlApp::OnKeyUp);

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

void WinUAPXamlApp::UpdateScreenSize(float32 width, float32 height)
{
    viewWidth = static_cast<int32>(width);
    viewHeight = static_cast<int32>(height);
    physicalWidth = static_cast<int32>(width * viewScaleX);
    physicalHeight = static_cast<int32>(height * viewScaleY);
}

void WinUAPXamlApp::UpdateScreenScale(float32 scaleX, float32 scaleY)
{
    viewScaleX = scaleX;
    viewScaleY = scaleY;
    physicalWidth = static_cast<int32>(viewWidth * viewScaleX);
    physicalHeight = static_cast<int32>(viewHeight * viewScaleY);
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

const wchar_t WinUAPXamlApp::xamlTextBoxStyles[] = LR"(
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
