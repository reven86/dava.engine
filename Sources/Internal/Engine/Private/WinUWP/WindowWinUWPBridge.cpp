#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/WindowWinUWPBridge.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Public/Window.h"
#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/WinUWP/WindowWinUWP.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowWinUWPBridge::WindowWinUWPBridge(WindowWinUWP* window)
    : uwpWindow(window)
{
}

void WindowWinUWPBridge::BindToXamlWindow(::Windows::UI::Xaml::Window ^ xamlWnd)
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

void WindowWinUWPBridge::AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl)
{
    xamlCanvas->Children->Append(xamlControl);
}

void WindowWinUWPBridge::RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl)
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

void WindowWinUWPBridge::PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y)
{
    xamlCanvas->SetLeft(xamlControl, x);
    xamlCanvas->SetTop(xamlControl, y);
}

void WindowWinUWPBridge::UnfocusXamlControl()
{
    // XAML controls cannot be unfocused programmatically, this is especially useful for text fields
    // So use dummy offscreen control that steals focus
    xamlControlThatStealsFocus->Focus(::Windows::UI::Xaml::FocusState::Pointer);
}

void WindowWinUWPBridge::TriggerPlatformEvents()
{
    using namespace ::Windows::UI::Core;
    xamlWindow->Dispatcher->TryRunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(this, &WindowWinUWPBridge::OnTriggerPlatformEvents));
}

void WindowWinUWPBridge::DoResizeWindow(float32 width, float32 height)
{
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::ViewManagement;

    // TODO: check width and height against zero, minimum window size and screen size
    ApplicationView ^ appView = ApplicationView::GetForCurrentView();
    appView->TryResizeView(Size(width, height));
}

void WindowWinUWPBridge::DoCloseWindow()
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

void WindowWinUWPBridge::OnTriggerPlatformEvents()
{
    uwpWindow->ProcessPlatformEvents();
}

void WindowWinUWPBridge::OnActivated(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::WindowActivatedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Core;
    bool hasFocus = arg->WindowActivationState != CoreWindowActivationState::Deactivated;
    uwpWindow->GetWindow()->PostFocusChanged(hasFocus);
}

void WindowWinUWPBridge::OnVisibilityChanged(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::VisibilityChangedEventArgs ^ arg)
{
    uwpWindow->GetWindow()->PostVisibilityChanged(arg->Visible);
}

void WindowWinUWPBridge::OnCharacterReceived(::Windows::UI::Core::CoreWindow ^ /*coreWindow*/, ::Windows::UI::Core::CharacterReceivedEventArgs ^ arg)
{
    uwpWindow->GetWindow()->PostKeyChar(arg->KeyCode, arg->KeyStatus.WasKeyDown);
}

void WindowWinUWPBridge::OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher ^ /*dispatcher*/, ::Windows::UI::Core::AcceleratorKeyEventArgs ^ arg)
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
        uwpWindow->GetWindow()->PostKeyDown(key, status.WasKeyDown);
        break;
    case CoreAcceleratorKeyEventType::KeyUp:
    case CoreAcceleratorKeyEventType::SystemKeyUp:
        uwpWindow->GetWindow()->PostKeyUp(key);
        break;
    default:
        break;
    }
}

void WindowWinUWPBridge::OnSizeChanged(::Platform::Object ^ /*sender*/, ::Windows::UI::Xaml::SizeChangedEventArgs ^ arg)
{
    float32 w = arg->NewSize.Width;
    float32 h = arg->NewSize.Height;
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    uwpWindow->GetWindow()->PostSizeChanged(w, h, scaleX, scaleY);
}

void WindowWinUWPBridge::OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel ^ /*panel*/, ::Platform::Object ^ /*obj*/)
{
    float32 w = static_cast<float32>(xamlSwapChainPanel->ActualWidth);
    float32 h = static_cast<float32>(xamlSwapChainPanel->ActualHeight);
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    uwpWindow->GetWindow()->PostSizeChanged(w, h, scaleX, scaleY);
}

void WindowWinUWPBridge::OnPointerPressed(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    DispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        std::bitset<5> state = FillMouseButtonState(prop);

        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;
        e.mclickEvent.button = GetMouseButtonIndex(state);
        uwpWindow->GetDispatcher()->PostEvent(e);

        mouseButtonState = state;
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        // TODO: implement later
    }
}

void WindowWinUWPBridge::OnPointerReleased(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    DispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;
        e.mclickEvent.button = GetMouseButtonIndex(mouseButtonState);
        uwpWindow->GetDispatcher()->PostEvent(e);

        mouseButtonState.reset();
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        // TODO: implement later
    }
}

void WindowWinUWPBridge::OnPointerMoved(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    DispatcherEvent e;
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
                e.type = state[i] ? DispatcherEvent::MOUSE_BUTTON_DOWN : DispatcherEvent::MOUSE_BUTTON_UP;
                e.mclickEvent.button = static_cast<uint32>(i + 1);
                uwpWindow->GetDispatcher()->PostEvent(e);
            }
        }

        e.type = DispatcherEvent::MOUSE_MOVE;
        e.mmoveEvent.x = pointerPoint->Position.X;
        e.mmoveEvent.y = pointerPoint->Position.Y;
        uwpWindow->GetDispatcher()->PostEvent(e);

        mouseButtonState = state;
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        // TODO: implement later
    }
}

void WindowWinUWPBridge::OnPointerWheelChanged(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);

    DispatcherEvent e;
    e.type = DispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = uwpWindow->GetWindow();
    e.mwheelEvent.x = pointerPoint->Position.X;
    e.mwheelEvent.y = pointerPoint->Position.Y;
    e.mwheelEvent.delta = pointerPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
    uwpWindow->GetDispatcher()->PostEvent(e);
}

uint32 WindowWinUWPBridge::GetMouseButtonIndex(::Windows::UI::Input::PointerPointProperties ^ props)
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

uint32 WindowWinUWPBridge::GetMouseButtonIndex(std::bitset<5> state)
{
    for (size_t i = 0, n = state.size(); i < n; ++i)
    {
        if (state[i])
            return static_cast<uint32>(i + 1);
    }
    return 0;
}

std::bitset<5> WindowWinUWPBridge::FillMouseButtonState(::Windows::UI::Input::PointerPointProperties ^ props)
{
    std::bitset<5> state;
    state.set(0, props->IsLeftButtonPressed);
    state.set(1, props->IsRightButtonPressed);
    state.set(2, props->IsMiddleButtonPressed);
    state.set(3, props->IsXButton1Pressed);
    state.set(4, props->IsXButton2Pressed);
    return state;
}

void WindowWinUWPBridge::CreateBaseXamlUI()
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

void WindowWinUWPBridge::InstallEventHandlers()
{
    using namespace ::Platform;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::UI::Xaml;
    using namespace ::Windows::UI::Xaml::Input;
    using namespace ::Windows::UI::Xaml::Controls;

    CoreWindow ^ coreWindow = xamlWindow->CoreWindow;

    tokenActivated = coreWindow->Activated += ref new TypedEventHandler<CoreWindow ^, WindowActivatedEventArgs ^>(this, &WindowWinUWPBridge::OnActivated);
    tokenVisibilityChanged = coreWindow->VisibilityChanged += ref new TypedEventHandler<CoreWindow ^, VisibilityChangedEventArgs ^>(this, &WindowWinUWPBridge::OnVisibilityChanged);

    tokenCharacterReceived = coreWindow->CharacterReceived += ref new TypedEventHandler<CoreWindow ^, CharacterReceivedEventArgs ^>(this, &WindowWinUWPBridge::OnCharacterReceived);
    tokenAcceleratorKeyActivated = xamlWindow->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher ^, AcceleratorKeyEventArgs ^>(this, &WindowWinUWPBridge::OnAcceleratorKeyActivated);

    tokenSizeChanged = xamlSwapChainPanel->SizeChanged += ref new SizeChangedEventHandler(this, &WindowWinUWPBridge::OnSizeChanged);
    tokenCompositionScaleChanged = xamlSwapChainPanel->CompositionScaleChanged += ref new TypedEventHandler<SwapChainPanel ^, Object ^>(this, &WindowWinUWPBridge::OnCompositionScaleChanged);

    tokenPointerPressed = xamlSwapChainPanel->PointerPressed += ref new PointerEventHandler(this, &WindowWinUWPBridge::OnPointerPressed);
    tokenPointerReleased = xamlSwapChainPanel->PointerReleased += ref new PointerEventHandler(this, &WindowWinUWPBridge::OnPointerReleased);
    tokenPointerMoved = xamlSwapChainPanel->PointerMoved += ref new PointerEventHandler(this, &WindowWinUWPBridge::OnPointerMoved);
    tokenPointerWheelChanged = xamlSwapChainPanel->PointerWheelChanged += ref new PointerEventHandler(this, &WindowWinUWPBridge::OnPointerWheelChanged);
}

void WindowWinUWPBridge::UninstallEventHandlers()
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

::Platform::String ^ WindowWinUWPBridge::xamlWorkaroundWebViewProblems = LR"(
<WebView x:Name="dummyWebView" Visibility="Collapsed"
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
</WebView>
)";

::Platform::String ^ WindowWinUWPBridge::xamlWorkaroundTextBoxProblems = LR"(
<TextBox x:Name="dummyTextBox" Visibility="Collapsed"
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" 
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
</TextBox>
)";

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
