#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/Window/WindowNativeBridgeUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* window)
    : uwpWindow(window)
{
}

void WindowNativeBridge::BindToXamlWindow(::Windows::UI::Xaml::Window ^ xamlWnd)
{
    DVASSERT(xamlWindow == nullptr);
    DVASSERT(xamlWnd != nullptr);

    xamlWindow = xamlWnd;

    CreateBaseXamlUI();
    InstallEventHandlers();

    float32 w = xamlWindow->Bounds.Width;
    float32 h = xamlWindow->Bounds.Height;
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    uwpWindow->GetWindow()->PostWindowCreated(uwpWindow, w, h, scaleX, scaleY);

    xamlWindow->Activate();
}

void WindowNativeBridge::AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl)
{
    xamlCanvas->Children->Append(xamlControl);
}

void WindowNativeBridge::RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl)
{
    unsigned int index = 0;
    for (auto x = xamlCanvas->Children->First(); x->HasCurrent; x->MoveNext(), ++index)
    {
        if (x->Current == xamlControl)
        {
            xamlCanvas->Children->RemoveAt(index);
            break;
        }
    }
}

void WindowNativeBridge::PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y)
{
    xamlCanvas->SetLeft(xamlControl, x);
    xamlCanvas->SetTop(xamlControl, y);
}

void WindowNativeBridge::UnfocusXamlControl()
{
    // XAML controls cannot be unfocused programmatically, this is especially useful for text fields
    // So use dummy offscreen control that steals focus
    xamlControlThatStealsFocus->Focus(::Windows::UI::Xaml::FocusState::Pointer);
}

void WindowNativeBridge::TriggerPlatformEvents()
{
    using namespace ::Windows::UI::Core;
    xamlWindow->Dispatcher->TryRunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(this, &WindowNativeBridge::OnTriggerPlatformEvents));
}

void WindowNativeBridge::DoResizeWindow(float32 width, float32 height)
{
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::ViewManagement;

    // TODO: check width and height against zero, minimum window size and screen size
    ApplicationView ^ appView = ApplicationView::GetForCurrentView();
    appView->TryResizeView(Size(width, height));
}

void WindowNativeBridge::DoCloseWindow()
{
    // WinRT does not permit to close main window, so for primary window pretend that window has been closed.
    // For secondary window invoke Close() method, and also do not wait Closed event as stated in MSDN:
    //      Apps are typically suspended, not terminated. As a result, this event (Closed) is rarely fired, if ever.
    if (!uwpWindow->GetWindow()->IsPrimary())
    {
        xamlWindow->CoreWindow->Close();
    }
    uwpWindow->GetWindow()->PostFocusChanged(false);
    uwpWindow->GetWindow()->PostVisibilityChanged(false);
    uwpWindow->GetWindow()->PostWindowDestroyed();
    UninstallEventHandlers();
}

void WindowNativeBridge::SetMouseMode(eMouseMode newMode)
{
    if (mouseMode == newMode)
    {
        return;
    }
    mouseMode = newMode;
    auto task = [this, newMode]() {
        nativeMouseMode = newMode;
        deferredMouseMode = false;
        switch (newMode)
        {
        case DAVA::eMouseMode::FRAME:
            //not implemented
            SetMouseCaptured(false);
            SetMouseVisibility(true);
            break;
        case DAVA::eMouseMode::PINNING:
        {
            if (hasFocus && !focusChanged)
            {
                SetMouseCaptured(true);
                SetMouseVisibility(false);
            }
            else
            {
                deferredMouseMode = true;
            }
            break;
        }
        case DAVA::eMouseMode::OFF:
        {
            SetMouseCaptured(false);
            SetMouseVisibility(true);
            break;
        }
        case DAVA::eMouseMode::HIDE:
        {
            SetMouseCaptured(false);
            SetMouseVisibility(false);
            break;
        }
        }
    };
    uwpWindow->RunAsyncOnUIThread(task);
}

void WindowNativeBridge::GetMouseMode() const
{
    return mouseMode;
}

void WindowNativeBridge::SetMouseVisibility(bool visible)
{
    // run on UI thread
    if (mouseVisibled == visible)
    {
        return;
    }
    mouseVisibled = visible;
    using ::Windows::UI::Core::CoreCursor;
    using ::Windows::UI::Core::CoreCursorType;
    if (visible)
    {
        ::Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
    }
    else
    {
        ::Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
    }
}

void WindowNativeBridge::SetMouseCaptured(bool capture)
{
    using namespace ::Windows::Devices::Input;
    using namespace ::Windows::Foundation;
    if (mouseCaptured == capture)
    {
        return;
    }
    mouseCaptured = capture;
    if (capture)
    {
        tokenMouseMoved = MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice ^, MouseEventArgs ^>(this, &WindowNativeBridge::OnMouseMoved);
    }
    else
    {
        MouseDevice::GetForCurrentView()->MouseMoved -= tokenMouseMoved;
    }
}

bool WindowNativeBridge::DeferredMouseMode(const MainDispatcherEvent& e)
{
    // run on UI thread
    if (!hasFocus)
    {
        return true;
    }
    focusChanged = false;
    if (deferredMouseMode)
    {
        if (MainDispatcherEvent::MOUSE_MOVE != e.type && MainDispatcherEvent::MOUSE_BUTTON_UP != e.type && MainDispatcherEvent::MOUSE_BUTTON_DOWN != e.type)
        {
            deferredMouseMode = false;
            SetMouseVisibility(false);
            if (eMouseMode::PINNING == nativeMouseMode)
            {
                SetMouseCaptured(true);
                skipMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
            }
            return false;
        }
        else if (MainDispatcherEvent::MOUSE_BUTTON_UP == e.type)
        {
            // check, only mouse release event in work rect tern on capture mode
            bool mclickInRect = true;
            Vector2 virtualPoint = VirtualCoordinatesSystem::Instance()->ConvertInputToVirtual(Vector2(e.mclickEvent.x, e.mclickEvent.y));
            mclickInRect &= (e.mclickEvent.x >= 0.f && e.mclickEvent.x <= uwpWindow->GetWindow()->GetWidth());
            mclickInRect &= (e.mclickEvent.y >= 0.f && e.mclickEvent.y <= uwpWindow->GetWindow()->GetHeight());
            if (mclickInRect && hasFocus)
            {
                deferredMouseMode = false;
                SetMouseVisibility(false);
                if (eMouseMode::PINNING == nativeMouseMode)
                {
                    SetMouseCaptured(true);
                    skipMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
                }
                // skip this event
            }
        }
        return true;
    }
    return false;
}

void WindowNativeBridge::OnTriggerPlatformEvents()
{
    uwpWindow->ProcessPlatformEvents();
}

void WindowNativeBridge::OnActivated(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::WindowActivatedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Core;
    hasFocus = arg->WindowActivationState != CoreWindowActivationState::Deactivated;
    if (!hasFocus)
    {
        focusChanged = true;
        if (eMouseMode::PINNING == nativeMouseMode)
        {
            SetMouseVisibility(true);
            SetMouseCaptured(false);
            deferredMouseMode = true;
        }
        else if (eMouseMode::HIDE == nativeMouseMode)
        {
            SetMouseVisibility(true);
            deferredMouseMode = true;
        }
    }
    uwpWindow->GetWindow()->PostFocusChanged(hasFocus);
}

void WindowNativeBridge::OnVisibilityChanged(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::VisibilityChangedEventArgs ^ arg)
{
    uwpWindow->GetWindow()->PostVisibilityChanged(arg->Visible);
}

void WindowNativeBridge::OnCharacterReceived(::Windows::UI::Core::CoreWindow ^ /*coreWindow*/, ::Windows::UI::Core::CharacterReceivedEventArgs ^ arg)
{
    uwpWindow->GetWindow()->PostKeyChar(arg->KeyCode, arg->KeyStatus.WasKeyDown);
}

void WindowNativeBridge::OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher ^ /*dispatcher*/, ::Windows::UI::Core::AcceleratorKeyEventArgs ^ arg)
{
    using namespace ::Windows::UI::Core;

    CorePhysicalKeyStatus status = arg->KeyStatus;
    uint32 key = static_cast<uint32>(arg->VirtualKey);
    if ((key == VK_SHIFT && status.ScanCode == 0x36) || status.IsExtendedKey)
    {
        key |= 0x100;
    }

    switch (arg->EventType)
    {
    case CoreAcceleratorKeyEventType::KeyDown:
    case CoreAcceleratorKeyEventType::SystemKeyDown:
    {
        MainDispatcherEvent e;
        e.type = MainDispatcherEvent::KEY_DOWN;
        if (!DeferredMouseMode(e))
        {
            uwpWindow->GetWindow()->PostKeyDown(key, status.WasKeyDown);
        }
        break;
    }
    case CoreAcceleratorKeyEventType::KeyUp:
    case CoreAcceleratorKeyEventType::SystemKeyUp:
    {
        MainDispatcherEvent e;
        e.type = MainDispatcherEvent::KEY_UP;
        if (!DeferredMouseMode(e))
        {
            uwpWindow->GetWindow()->PostKeyUp(key);
        }
        break;
    }
    default:
        break;
    }
}

void WindowNativeBridge::OnSizeChanged(::Platform::Object ^ /*sender*/, ::Windows::UI::Xaml::SizeChangedEventArgs ^ arg)
{
    float32 w = arg->NewSize.Width;
    float32 h = arg->NewSize.Height;
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    uwpWindow->GetWindow()->PostSizeChanged(w, h, scaleX, scaleY);
}

void WindowNativeBridge::OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel ^ /*panel*/, ::Platform::Object ^ /*obj*/)
{
    float32 w = static_cast<float32>(xamlSwapChainPanel->ActualWidth);
    float32 h = static_cast<float32>(xamlSwapChainPanel->ActualHeight);
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    uwpWindow->GetWindow()->PostSizeChanged(w, h, scaleX, scaleY);
}

void WindowNativeBridge::OnPointerPressed(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    MainDispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        std::bitset<5> state = FillMouseButtonState(prop);

        e.type = MainDispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;
        e.mclickEvent.button = GetMouseButtonIndex(state);

        if (!DeferredMouseMode(e))
        {
            uwpWindow->GetDispatcher()->PostEvent(e);
        }

        mouseButtonState = state;
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        // TODO: implement later
    }
}

void WindowNativeBridge::OnPointerReleased(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    MainDispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        e.type = MainDispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;
        e.mclickEvent.button = GetMouseButtonIndex(mouseButtonState);

        if (!DeferredMouseMode(e))
        {
            uwpWindow->GetDispatcher()->PostEvent(e);
        }

        mouseButtonState.reset();
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        // TODO: implement later
    }
}

void WindowNativeBridge::OnPointerMoved(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    MainDispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        std::bitset<5> state = FillMouseButtonState(prop);
        std::bitset<5> change = mouseButtonState ^ state;

        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;

        for (size_t i = 0, n = change.size(); i < n; ++i)
        {
            if (change[i])
            {
                e.type = state[i] ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
                e.mclickEvent.button = static_cast<uint32>(i + 1);
                if (!DeferredMouseMode(e))
                {
                    uwpWindow->GetDispatcher()->PostEvent(e);
                }
            }
        }

        e.type = MainDispatcherEvent::MOUSE_MOVE;
        e.mmoveEvent.x = pointerPoint->Position.X;
        e.mmoveEvent.y = pointerPoint->Position.Y;

        if (!DeferredMouseMode(e))
        {
            uwpWindow->GetDispatcher()->PostEvent(e);
        }

        mouseButtonState = state;
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        // TODO: implement later
    }
}

void WindowNativeBridge::OnPointerWheelChanged(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);

    MainDispatcherEvent e;
    e.type = MainDispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();
    e.mwheelEvent.x = pointerPoint->Position.X;
    e.mwheelEvent.y = pointerPoint->Position.Y;
    e.mwheelEvent.deltaX = 0.0f;
    e.mwheelEvent.deltaY = static_cast<float32>(pointerPoint->Properties->MouseWheelDelta / WHEEL_DELTA);
    if (!DeferredMouseMode(e))
    {
        uwpWindow->GetDispatcher()->PostEvent(e);
    }
}

void WindowNativeBridge::OnMouseMoved(Windows::Devices::Input::MouseDevice ^ mouseDevice, Windows::Devices::Input::MouseEventArgs ^ args)
{
    if (eMouseMode::PINNING != nativeMouseMode)
    {
        return;
    }
    MainDispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();
    e.type = MainDispatcherEvent::MOUSE_MOVE;
    e.mmoveEvent.x = static_cast<float32>(args->MouseDelta.X);
    e.mmoveEvent.y = static_cast<float32>(args->MouseDelta.Y);
    // after enabled Pinning mode, skip move events, large x, y delta
    if (skipMouseMoveEvents)
    {
        skipMouseMoveEvents--;
        return;
    }
    if (!DeferredMouseMode(e))
    {
        uwpWindow->GetDispatcher()->PostEvent(e);
    }
}

uint32 WindowNativeBridge::GetMouseButtonIndex(::Windows::UI::Input::PointerPointProperties ^ props)
{
    if (props->IsLeftButtonPressed)
        return 1;
    if (props->IsRightButtonPressed)
        return 2;
    if (props->IsMiddleButtonPressed)
        return 3;
    if (props->IsXButton1Pressed)
        return 4;
    if (props->IsXButton2Pressed)
        return 5;
    return 0;
}

uint32 WindowNativeBridge::GetMouseButtonIndex(std::bitset<5> state)
{
    for (size_t i = 0, n = state.size(); i < n; ++i)
    {
        if (state[i])
            return static_cast<uint32>(i + 1);
    }
    return 0;
}

std::bitset<5> WindowNativeBridge::FillMouseButtonState(::Windows::UI::Input::PointerPointProperties ^ props)
{
    std::bitset<5> state;
    state.set(0, props->IsLeftButtonPressed);
    state.set(1, props->IsRightButtonPressed);
    state.set(2, props->IsMiddleButtonPressed);
    state.set(3, props->IsXButton1Pressed);
    state.set(4, props->IsXButton2Pressed);
    return state;
}

void WindowNativeBridge::CreateBaseXamlUI()
{
    using ::Windows::UI::Xaml::Markup::XamlReader;
    using ::Windows::UI::Xaml::ResourceDictionary;
    using namespace ::Windows::UI::Xaml::Controls;

    xamlCanvas = ref new Canvas;

    xamlSwapChainPanel = ref new SwapChainPanel;
    xamlSwapChainPanel->Children->Append(xamlCanvas);

    {
        // Universal Windows Platfrom has some problems and bugs when application works with XAML UI:
        //  - MouseDevice::MouseMoved events are lost on Surface devices
        //  - exception is thrown when inserting some text into programmatically created TextBox
        // Solution to resolve these problems is to create invisible dummy WebView and TextBox controls
        // from XAML sheet and add them to control hierarchy
        using ::Windows::UI::Xaml::Controls::TextBox;
        WebView ^ dummyWebView = static_cast<WebView ^>(XamlReader::Load(xamlWorkaroundWebViewProblems));
        TextBox ^ dummyTextBox = static_cast<TextBox ^>(XamlReader::Load(xamlWorkaroundTextBoxProblems));

        AddXamlControl(dummyWebView);
        AddXamlControl(dummyTextBox);
    }

    // Windows UAP doesn't allow to unfocus UI control programmatically
    // It only permits to set focus at another control
    // So create dummy offscreen button that steals focus when there is
    // a need to unfocus native control, especially useful for text fields
    xamlControlThatStealsFocus = ref new Button();
    xamlControlThatStealsFocus->Content = L"I steal your focus";
    xamlControlThatStealsFocus->Width = 30;
    xamlControlThatStealsFocus->Height = 20;
    AddXamlControl(xamlControlThatStealsFocus);
    PositionXamlControl(xamlControlThatStealsFocus, -1000.0f, -1000.0f);

    xamlWindow->Content = xamlSwapChainPanel;
}

void WindowNativeBridge::InstallEventHandlers()
{
    using namespace ::Platform;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::UI::Xaml;
    using namespace ::Windows::UI::Xaml::Input;
    using namespace ::Windows::Devices::Input;
    using namespace ::Windows::UI::Xaml::Controls;

    CoreWindow ^ coreWindow = xamlWindow->CoreWindow;

    tokenActivated = coreWindow->Activated += ref new TypedEventHandler<CoreWindow ^, WindowActivatedEventArgs ^>(this, &WindowNativeBridge::OnActivated);
    tokenVisibilityChanged = coreWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow ^, VisibilityChangedEventArgs ^>(this, &WindowNativeBridge::OnVisibilityChanged);

    tokenCharacterReceived = coreWindow->CharacterReceived += ref new TypedEventHandler<CoreWindow ^, CharacterReceivedEventArgs ^>(this, &WindowNativeBridge::OnCharacterReceived);
    tokenAcceleratorKeyActivated = xamlWindow->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher ^, AcceleratorKeyEventArgs ^>(this, &WindowNativeBridge::OnAcceleratorKeyActivated);

    tokenSizeChanged = xamlSwapChainPanel->SizeChanged += ref new SizeChangedEventHandler(this, &WindowNativeBridge::OnSizeChanged);
    tokenCompositionScaleChanged = xamlSwapChainPanel->CompositionScaleChanged += ref new TypedEventHandler<SwapChainPanel ^, Object ^>(this, &WindowNativeBridge::OnCompositionScaleChanged);

    tokenPointerPressed = xamlSwapChainPanel->PointerPressed += ref new PointerEventHandler(this, &WindowNativeBridge::OnPointerPressed);
    tokenPointerReleased = xamlSwapChainPanel->PointerReleased += ref new PointerEventHandler(this, &WindowNativeBridge::OnPointerReleased);
    tokenPointerMoved = xamlSwapChainPanel->PointerMoved += ref new PointerEventHandler(this, &WindowNativeBridge::OnPointerMoved);
    tokenPointerWheelChanged = xamlSwapChainPanel->PointerWheelChanged += ref new PointerEventHandler(this, &WindowNativeBridge::OnPointerWheelChanged);
    //     tokenMouseMoved = MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice ^, MouseEventArgs ^>(this, &WindowNativeBridge::OnMouseMoved);
}

void WindowNativeBridge::UninstallEventHandlers()
{
    using namespace ::Windows::UI::Core;
    CoreWindow ^ coreWindow = xamlWindow->CoreWindow;

    coreWindow->Activated -= tokenActivated;
    coreWindow->VisibilityChanged -= tokenVisibilityChanged;

    coreWindow->CharacterReceived -= tokenCharacterReceived;
    xamlWindow->Dispatcher->AcceleratorKeyActivated -= tokenAcceleratorKeyActivated;

    xamlSwapChainPanel->SizeChanged -= tokenSizeChanged;
    xamlSwapChainPanel->CompositionScaleChanged -= tokenCompositionScaleChanged;

    xamlSwapChainPanel->PointerPressed -= tokenPointerPressed;
    xamlSwapChainPanel->PointerReleased -= tokenPointerReleased;
    xamlSwapChainPanel->PointerMoved -= tokenPointerMoved;
    xamlSwapChainPanel->PointerWheelChanged -= tokenPointerWheelChanged;
}

::Platform::String ^ WindowNativeBridge::xamlWorkaroundWebViewProblems = LR"(
<WebView x:Name="dummyWebView" Visibility="Collapsed"
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
</WebView>
)";

::Platform::String ^ WindowNativeBridge::xamlWorkaroundTextBoxProblems = LR"(
<TextBox x:Name="dummyTextBox" Visibility="Collapsed"
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
</TextBox>
)";

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
