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

void WindowNativeBridge::BindToXamlWindow(::Windows::UI::Xaml::Window ^ xamlWindow_)
{
    using ::Windows::Foundation::Size;
    using ::Windows::UI::ViewManagement::ApplicationView;

    DVASSERT(xamlWindow == nullptr);
    DVASSERT(xamlWindow_ != nullptr);

    xamlWindow = xamlWindow_;

    // Limit minimum window size to some reasonable value
    ApplicationView::GetForCurrentView()->SetPreferredMinSize(Size(128, 128));

    CreateBaseXamlUI();
    InstallEventHandlers();

    float32 w = xamlWindow->Bounds.Width;
    float32 h = xamlWindow->Bounds.Height;
    float32 surfW = w * xamlSwapChainPanel->CompositionScaleX * surfaceScale;
    float32 surfH = h * xamlSwapChainPanel->CompositionScaleY * surfaceScale;
    float32 dpi = ::Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi;
    eFullscreen fullscreen = ApplicationView::GetForCurrentView()->IsFullScreenMode ? eFullscreen::On : eFullscreen::Off;
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCreatedEvent(window, w, h, surfW, surfH, dpi, fullscreen));

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

float32 WindowNativeBridge::GetSurfaceScale() const
{
    return surfaceScale;
}

void WindowNativeBridge::SetSurfaceScale(float32 scale)
{
    surfaceScale = scale;

    const float32 width = static_cast<float32>(xamlSwapChainPanel->ActualWidth);
    const float32 height = static_cast<float32>(xamlSwapChainPanel->ActualHeight);
    const float32 surfaceWidth = width * xamlSwapChainPanel->CompositionScaleX * surfaceScale;
    const float32 surfaceHeight = height * xamlSwapChainPanel->CompositionScaleY * surfaceScale;

    bool isFullscreen = ::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->IsFullScreenMode;
    eFullscreen fullscreen = isFullscreen ? eFullscreen::On : eFullscreen::Off;

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, width, height, surfaceWidth, surfaceHeight, fullscreen));
}

void WindowNativeBridge::SetFullscreen(eFullscreen newMode)
{
    using ::Windows::UI::ViewManagement::ApplicationView;
    bool isFullscreenRequested = newMode == eFullscreen::On;

    ApplicationView ^ view = ApplicationView::GetForCurrentView();
    if (isFullscreenRequested == view->IsFullScreenMode)
    {
        return;
    }

    if (isFullscreenRequested)
    {
        view->TryEnterFullScreenMode();
    }
    else
    {
        view->ExitFullScreenMode();
    }
}

void WindowNativeBridge::SetCursorVisibility(bool visible)
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
            mouseMoveSkipCount = SKIP_N_MOUSE_MOVE_EVENTS;
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
    if (!hasFocus)
    {
        if (captureMode == eCursorCapture::PINNING)
        {
            SetCursorCapture(eCursorCapture::OFF);
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowCaptureLostEvent(window));
        }
        SetCursorVisibility(true);
    }
}

void WindowNativeBridge::OnVisibilityChanged(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::VisibilityChangedEventArgs ^ arg)
{
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowVisibilityChangedEvent(window, arg->Visible));
}

void WindowNativeBridge::OnCharacterReceived(::Windows::UI::Core::CoreWindow ^ /*coreWindow*/, ::Windows::UI::Core::CharacterReceivedEventArgs ^ arg)
{
    eModifierKeys modifierKeys = GetModifierKeys();
    // Windows translates some Ctrl key combinations into ASCII control characters.
    // It seems to me that control character are not wanted by game to handle in character message.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/gg153546(v=vs.85).aspx
    if ((modifierKeys & eModifierKeys::CONTROL) == eModifierKeys::NONE)
    {
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, MainDispatcherEvent::KEY_CHAR, arg->KeyCode, modifierKeys, arg->KeyStatus.WasKeyDown));
    }
}

void WindowNativeBridge::OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher ^ /*dispatcher*/, ::Windows::UI::Core::AcceleratorKeyEventArgs ^ arg)
{
    using ::Windows::System::VirtualKey;
    using namespace ::Windows::UI::Core;

    // Process only KeyDown/KeyUp and SystemKeyDown/SystemKeyUp event types to skip unwanted messages, such as
    // Character (handled in OnCharacterReceived), DeadCharacter, SystemCharacter, etc.
    bool isPressed = false;
    CoreAcceleratorKeyEventType eventType = arg->EventType;
    switch (eventType)
    {
    case CoreAcceleratorKeyEventType::KeyDown:
    case CoreAcceleratorKeyEventType::SystemKeyDown:
        isPressed = true; // Fall through below
    case CoreAcceleratorKeyEventType::KeyUp:
    case CoreAcceleratorKeyEventType::SystemKeyUp:
    {
        const unsigned int RSHIFT_SCANCODE = 0x36;

        CorePhysicalKeyStatus status = arg->KeyStatus;
        uint32 key = static_cast<uint32>(arg->VirtualKey);
        // Ups, UWP does not support MapVirtualKey function to distinguish left and right shift
        if (status.IsExtendedKey || (arg->VirtualKey == VirtualKey::Shift && status.ScanCode == RSHIFT_SCANCODE))
        {
            key |= 0x100;
        }

        eModifierKeys modifierKeys = GetModifierKeys();
        MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::KEY_DOWN : MainDispatcherEvent::KEY_UP;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(window, type, key, modifierKeys, status.WasKeyDown));
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

    float32 surfW = w * xamlSwapChainPanel->CompositionScaleX * surfaceScale;
    float32 surfH = h * xamlSwapChainPanel->CompositionScaleY * surfaceScale;

    bool isFullscreen = ::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->IsFullScreenMode;
    eFullscreen fullscreen = isFullscreen ? eFullscreen::On : eFullscreen::Off;

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, surfW, surfH, fullscreen));
}

void WindowNativeBridge::OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel ^ /*panel*/, ::Platform::Object ^ /*obj*/)
{
    using Windows::UI::ViewManagement::ApplicationView;

    float32 w = static_cast<float32>(xamlSwapChainPanel->ActualWidth);
    float32 h = static_cast<float32>(xamlSwapChainPanel->ActualHeight);
    float32 surfW = w * xamlSwapChainPanel->CompositionScaleX * surfaceScale;
    float32 surfH = h * xamlSwapChainPanel->CompositionScaleY * surfaceScale;
    float32 dpi = ::Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi;
    bool isFullscreen = ApplicationView::GetForCurrentView()->IsFullScreenMode;
    eFullscreen fullscreen = isFullscreen ? eFullscreen::On : eFullscreen::Off;

    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowSizeChangedEvent(window, w, h, surfW, surfH, fullscreen));
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowDpiChangedEvent(window, dpi));
}

void WindowNativeBridge::OnPointerPressed(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    eModifierKeys modifierKeys = GetModifierKeys();
    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    if (deviceType == PointerDeviceType::Mouse)
    {
        bool isPressed = false;
        eMouseButtons button = GetMouseButtonState(prop->PointerUpdateKind, &isPressed);
        bool isRelative = (captureMode == eCursorCapture::PINNING);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, MainDispatcherEvent::MOUSE_BUTTON_DOWN, button, x, y, 1, modifierKeys, isRelative));
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        uint32 touchId = pointerPoint->PointerId;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_DOWN, touchId, x, y, modifierKeys));
    }
}

void WindowNativeBridge::OnPointerReleased(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    eModifierKeys modifierKeys = GetModifierKeys();
    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    if (deviceType == PointerDeviceType::Mouse)
    {
        bool isPressed = false;
        eMouseButtons button = GetMouseButtonState(prop->PointerUpdateKind, &isPressed);
        bool isRelative = (captureMode == eCursorCapture::PINNING);
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, MainDispatcherEvent::MOUSE_BUTTON_UP, button, x, y, 1, modifierKeys, isRelative));
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        uint32 touchId = pointerPoint->PointerId;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_UP, touchId, x, y, modifierKeys));
    }
}

void WindowNativeBridge::OnPointerMoved(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    eModifierKeys modifierKeys = GetModifierKeys();
    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    if (deviceType == PointerDeviceType::Mouse)
    {
        if (prop->PointerUpdateKind != PointerUpdateKind::Other)
        {
            // First mouse button down (and last mouse button up) comes through OnPointerPressed/OnPointerReleased, other mouse clicks come here
            bool isPressed = false;
            eMouseButtons button = GetMouseButtonState(prop->PointerUpdateKind, &isPressed);
            MainDispatcherEvent::eType type = isPressed ? MainDispatcherEvent::MOUSE_BUTTON_DOWN : MainDispatcherEvent::MOUSE_BUTTON_UP;
            mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(window, type, button, x, y, 1, modifierKeys, false));
        }
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, modifierKeys, false));
    }
    else if (deviceType == PointerDeviceType::Touch)
    {
        uint32 touchId = pointerPoint->PointerId;
        mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowTouchEvent(window, MainDispatcherEvent::TOUCH_MOVE, touchId, x, y, modifierKeys));
    }
}

void WindowNativeBridge::OnPointerWheelChanged(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg)
{
    using namespace ::Windows::UI::Input;

    PointerPoint ^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties ^ prop = pointerPoint->Properties;

    float32 x = pointerPoint->Position.X;
    float32 y = pointerPoint->Position.Y;
    float32 deltaX = 0.f;
    float32 deltaY = static_cast<float32>(prop->MouseWheelDelta / WHEEL_DELTA);
    if (prop->IsHorizontalMouseWheel)
    {
        using std::swap;
        std::swap(deltaX, deltaY);
    }
    eModifierKeys modifierKeys = GetModifierKeys();
    bool isRelative = (captureMode == eCursorCapture::PINNING);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseWheelEvent(window, x, y, deltaX, deltaY, modifierKeys, isRelative));
}

void WindowNativeBridge::OnMouseMoved(Windows::Devices::Input::MouseDevice ^ mouseDevice, Windows::Devices::Input::MouseEventArgs ^ args)
{
    if (mouseMoveSkipCount)
    {
        mouseMoveSkipCount--;
        return;
    }
    float32 x = static_cast<float32>(args->MouseDelta.X);
    float32 y = static_cast<float32>(args->MouseDelta.Y);
    mainDispatcher->PostEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(window, x, y, GetModifierKeys(), true));
}

eModifierKeys WindowNativeBridge::GetModifierKeys() const
{
    using ::Windows::System::VirtualKey;
    using ::Windows::UI::Core::CoreWindow;
    using ::Windows::UI::Core::CoreVirtualKeyStates;

    eModifierKeys result = eModifierKeys::NONE;
    CoreWindow ^ coreWindow = xamlWindow->CoreWindow;

    CoreVirtualKeyStates keyState = coreWindow->GetKeyState(VirtualKey::Shift);
    if ((keyState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down)
    {
        result |= eModifierKeys::SHIFT;
    }

    keyState = coreWindow->GetKeyState(VirtualKey::Control);
    if ((keyState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down)
    {
        result |= eModifierKeys::CONTROL;
    }

    keyState = coreWindow->GetKeyState(VirtualKey::Menu);
    if ((keyState & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down)
    {
        result |= eModifierKeys::ALT;
    }
    return result;
}

eMouseButtons WindowNativeBridge::GetMouseButtonState(::Windows::UI::Input::PointerUpdateKind buttonUpdateKind, bool* isPressed)
{
    using ::Windows::UI::Input::PointerUpdateKind;

    *isPressed = false;
    switch (buttonUpdateKind)
    {
    case PointerUpdateKind::LeftButtonPressed:
        *isPressed = true;
    case PointerUpdateKind::LeftButtonReleased:
        return eMouseButtons::LEFT;
    case PointerUpdateKind::RightButtonPressed:
        *isPressed = true;
    case PointerUpdateKind::RightButtonReleased:
        return eMouseButtons::RIGHT;
    case PointerUpdateKind::MiddleButtonPressed:
        *isPressed = true;
    case PointerUpdateKind::MiddleButtonReleased:
        return eMouseButtons::MIDDLE;
    case PointerUpdateKind::XButton1Pressed:
        *isPressed = true;
    case PointerUpdateKind::XButton1Released:
        return eMouseButtons::EXTENDED1;
    case PointerUpdateKind::XButton2Pressed:
        *isPressed = true;
    case PointerUpdateKind::XButton2Released:
        return eMouseButtons::EXTENDED2;
    default:
        return eMouseButtons::NONE;
    }
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
    SetCursorCapture(eCursorCapture::OFF);
    SetCursorVisibility(true);
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
