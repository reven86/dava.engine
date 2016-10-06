#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/Window/WindowNativeBridgeUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Window.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
WindowNativeBridge::WindowNativeBridge(WindowBackend* windowBackend)
    : windowBackend(windowBackend)
    , window(windowBackend->window)
    , mainDispatcher(windowBackend->mainDispatcher)
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
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, w, h, scaleX, scaleY));

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

void WindowNativeBridge::ResizeWindow(float32 width, float32 height)
{
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::ViewManagement;

    // TODO: check width and height against zero, minimum window size and screen size
    ApplicationView ^ appView = ApplicationView::GetForCurrentView();
    appView->TryResizeView(Size(width, height));
}

void WindowNativeBridge::CloseWindow()
{
    // WinRT does not permit to close main window, so for primary window pretend that window has been closed.
    // For secondary window invoke Close() method, and also do not wait Closed event as stated in MSDN:
    //      Apps are typically suspended, not terminated. As a result, this event (Closed) is rarely fired, if ever.
    if (!window->IsPrimary())
    {
        xamlWindow->CoreWindow->Close();
    }

    UninstallEventHandlers();
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, false));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, false));
    mainDispatcher->SendEvent(MainDispatcherEvent::CreateWindowDestroyedEvent(window));
}

void WindowNativeBridge::SetTitle(const char8* title)
{
    using ::Windows::UI::ViewManagement::ApplicationView;

    WideString wideTitle = UTF8Utils::EncodeToWideString(title);
    ApplicationView::GetForCurrentView()->Title = ref new ::Platform::String(wideTitle.c_str());
}

void WindowNativeBridge::SetCursorVisible(bool visible)
{
    if (mouseVisible != visible)
    {
        using ::Windows::UI::Core::CoreCursor;
        using ::Windows::UI::Core::CoreCursorType;
        using ::Windows::UI::Core::CoreWindow;
        mouseVisible = visible;
        if (visible)
        {
            CoreWindow::GetForCurrentThread()->PointerCursor = defaultCursor;
        }
        else
        {
            CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
        }
    }
}

void WindowNativeBridge::SetCursorCapture(eCursorCapture mode)
{
    if (captureMode != mode)
    {
        using namespace ::Windows::Devices::Input;
        using namespace ::Windows::Foundation;
        captureMode = mode;
        switch (captureMode)
        {
        case DAVA::eCursorCapture::OFF:
            MouseDevice::GetForCurrentView()->MouseMoved -= tokenMouseMoved;
            break;
        case DAVA::eCursorCapture::FRAME:
            // now, not implemented
            break;
        case DAVA::eCursorCapture::PINNING:
            tokenMouseMoved = MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice ^, MouseEventArgs ^>(this, &WindowNativeBridge::OnMouseMoved);
            // after enabled Pinning mode, skip move events, large x, y delta
            skipNumberMouseMoveEvents = SKIP_N_MOUSE_MOVE_EVENTS;
            break;
        }
    }
}

void WindowNativeBridge::OnTriggerPlatformEvents()
{
    windowBackend->ProcessPlatformEvents();
}

void WindowNativeBridge::OnActivated(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::WindowActivatedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Core;
    bool hasFocus = arg->WindowActivationState != CoreWindowActivationState::Deactivated;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowFocusChangedEvent(window, hasFocus));
}

void WindowNativeBridge::OnVisibilityChanged(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::VisibilityChangedEventArgs ^ arg)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, arg->Visible));
}

void WindowNativeBridge::OnCharacterReceived(::Windows::UI::Core::CoreWindow ^ /*coreWindow*/, ::Windows::UI::Core::CharacterReceivedEventArgs ^ arg)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, arg->KeyCode, arg->KeyStatus.WasKeyDown));
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

    bool isPressed = arg->EventType == CoreAcceleratorKeyEventType::KeyDown ||
    arg->EventType == CoreAcceleratorKeyEventType::SystemKeyDown;
    MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, status.WasKeyDown));
}

void WindowNativeBridge::OnSizeChanged(::Platform::Object ^ /*sender*/, ::Windows::UI::Xaml::SizeChangedEventArgs ^ arg)
{
    float32 w = arg->NewSize.Width;
    float32 h = arg->NewSize.Height;
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, scaleX, scaleY));
}

void WindowNativeBridge::OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel ^ /*panel*/, ::Platform::Object ^ /*obj*/)
{
    float32 w = static_cast<float32>(xamlSwapChainPanel->ActualWidth);
    float32 h = static_cast<float32>(xamlSwapChainPanel->ActualHeight);
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, scaleX, scaleY));
}

void WindowNativeBridge::OnPointerPressed(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        std::bitset<5> state = FillMouseButtonState(prop);

        float32 x = pointerPoint->Position.X;
        float32 y = pointerPoint->Position.Y;
        uint32 button = GetMouseButtonIndex(state);
        bool isRelative = (captureMode == eCursorCapture::PINNING);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window,
                                                                                   MainDispatcherEvent::MOUSE_BUTTON_DOWN,
                                                                                   button,
                                                                                   x,
                                                                                   y,
                                                                                   1,
                                                                                   isRelative));
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

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        float32 x = pointerPoint->Position.X;
        float32 y = pointerPoint->Position.Y;
        uint32 button = GetMouseButtonIndex(mouseButtonState);
        bool isRelative = (captureMode == eCursorCapture::PINNING);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window,
                                                                                   MainDispatcherEvent::MOUSE_BUTTON_UP,
                                                                                   button,
                                                                                   x,
                                                                                   y,
                                                                                   1,
                                                                                   isRelative));
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

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        std::bitset<5> state = FillMouseButtonState(prop);
        std::bitset<5> change = mouseButtonState ^ state;

        float32 x = pointerPoint->Position.X;
        float32 y = pointerPoint->Position.Y;
        MainDispatcherEvent e = MainDispatcherEvent::CreateWindowMouseClickEvent(window,
                                                                                 MainDispatcherEvent::MOUSE_BUTTON_DOWN,
                                                                                 0,
                                                                                 x,
                                                                                 y,
                                                                                 1,
                                                                                 false);
        for (size_t i = 0, n = change.size(); i < n; ++i)
        {
            if (change[i])
            {
                e.type = state[i] ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
                e.mouseEvent.button = static_cast<uint32>(i + 1);
                mainDispatcher->PostEvent(e);
            }
        }

        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, false));
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

    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    float32 deltaY = static_cast<float32>(pointerPoint->Properties->MouseWheelDelta / WHEEL_DELTA);
    bool isRelative = (captureMode == eCursorCapture::PINNING);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, 0.f, deltaY, isRelative));
}

void WindowNativeBridge::OnMouseMoved(Windows::Devices::Input::MouseDevice ^ mouseDevice, Windows::Devices::Input::MouseEventArgs ^ args)
{
    if (skipNumberMouseMoveEvents)
    {
        skipNumberMouseMoveEvents--;
        return;
    }
    float32 x = static_cast<float32>(args->MouseDelta.X);
    float32 y = static_cast<float32>(args->MouseDelta.Y);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, true));
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
}

void WindowNativeBridge::UninstallEventHandlers()
{
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::Devices::Input;
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
    MouseDevice::GetForCurrentView()->MouseMoved -= tokenMouseMoved;
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
