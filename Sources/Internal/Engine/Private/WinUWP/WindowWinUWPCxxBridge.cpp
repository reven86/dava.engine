#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/WindowWinUWPCxxBridge.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/Dispatcher/Dispatcher.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/WinUWP/WindowWinUWP.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
// clang-format off

WindowWinUWPCxxBridge::WindowWinUWPCxxBridge(WindowWinUWP* window_)
    : window(window_)
{

}

void WindowWinUWPCxxBridge::RunAsyncOnUIThread(const Function<void()>& task)
{
    // add to queue
}

void WindowWinUWPCxxBridge::BindToXamlWindow(::Windows::UI::Xaml::Window^ xamlWindow_)
{
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Xaml::Controls;

    xamlWindow = xamlWindow_;

    xamlSwapChainPanel = ref new SwapChainPanel;
    xamlCanvas = ref new Canvas;

    xamlSwapChainPanel->Children->Append(xamlCanvas);
    xamlWindow->Content = xamlSwapChainPanel;

    xamlSwapChainPanel->MinWidth = 960.0;
    xamlSwapChainPanel->MinHeight = 640.0;

    InstallEventHandlers();

    Rect rc = xamlWindow->Bounds;

    float32 w = rc.Width;
    float32 h = rc.Height;
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    window->window->PostWindowCreated(window, w, h, scaleX, scaleY);

    xamlWindow->Activate();
}

void WindowWinUWPCxxBridge::OnActivated(::Platform::Object^ /*sender*/, ::Windows::UI::Core::WindowActivatedEventArgs^ arg)
{
    using namespace ::Windows::UI::Core;
    bool hasFocus = arg->WindowActivationState != CoreWindowActivationState::Deactivated;
    window->window->PostFocusChanged(hasFocus);
}

void WindowWinUWPCxxBridge::OnVisibilityChanged(::Platform::Object^ /*sender*/, ::Windows::UI::Core::VisibilityChangedEventArgs^ arg)
{
    window->window->PostVisibilityChanged(arg->Visible);
}

void WindowWinUWPCxxBridge::OnCharacterReceived(::Windows::UI::Core::CoreWindow^ /*coreWindow*/, ::Windows::UI::Core::CharacterReceivedEventArgs^ arg)
{
    DispatcherEvent e;
    e.type = DispatcherEvent::KEY_CHAR;
    e.window = window->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = arg->KeyCode;
    e.keyEvent.isRepeated = arg->KeyStatus.WasKeyDown;
    window->dispatcher->PostEvent(e);
}

void WindowWinUWPCxxBridge::OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher^ /*dispatcher*/, ::Windows::UI::Core::AcceleratorKeyEventArgs^ arg)
{
    using namespace ::Windows::UI::Core;

    CorePhysicalKeyStatus status = arg->KeyStatus;
    bool isPressed = arg->EventType == CoreAcceleratorKeyEventType::KeyDown || arg->EventType == CoreAcceleratorKeyEventType::SystemKeyDown;

    uint32 key = static_cast<uint32>(arg->VirtualKey);
    if ((key == VK_SHIFT && status.ScanCode == 0x36) || status.IsExtendedKey)
    {
        key |= 0x100;
    }

    DispatcherEvent e;
    e.type = isPressed ? DispatcherEvent::KEY_DOWN : DispatcherEvent::KEY_UP;
    e.window = window->window;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.keyEvent.key = key;
    e.keyEvent.isRepeated = status.WasKeyDown;
    window->dispatcher->PostEvent(e);
}

void WindowWinUWPCxxBridge::OnSizeChanged(::Platform::Object^ /*sender*/, ::Windows::UI::Xaml::SizeChangedEventArgs^ arg)
{
    float32 w = arg->NewSize.Width;
    float32 h = arg->NewSize.Height;
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    window->window->PostSizeChanged(w, h, scaleX, scaleY);
}

void WindowWinUWPCxxBridge::OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel^ /*panel*/, ::Platform::Object^ /*obj*/)
{
    float32 w = static_cast<float32>(xamlSwapChainPanel->ActualWidth);
    float32 h = static_cast<float32>(xamlSwapChainPanel->ActualHeight);
    float32 scaleX = xamlSwapChainPanel->CompositionScaleX;
    float32 scaleY = xamlSwapChainPanel->CompositionScaleY;
    window->window->PostSizeChanged(w, h, scaleX, scaleY);
}

void WindowWinUWPCxxBridge::OnPointerPressed(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    DispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window->window;

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        std::bitset<5> state = FillMouseButtonState(prop);

        e.type = DispatcherEvent::MOUSE_BUTTON_DOWN;
        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;
        e.mclickEvent.button = GetMouseButtonIndex(state);
        window->dispatcher->PostEvent(e);

        mouseButtonState = state;
    }
    else if (deviceType == PointerDeviceType::Touch)
    {

    }
}

void WindowWinUWPCxxBridge::OnPointerReleased(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    DispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window->window;

    if (deviceType == PointerDeviceType::Mouse || deviceType == PointerDeviceType::Pen)
    {
        e.type = DispatcherEvent::MOUSE_BUTTON_UP;
        e.mclickEvent.clicks = 1;
        e.mclickEvent.x = pointerPoint->Position.X;
        e.mclickEvent.y = pointerPoint->Position.Y;
        e.mclickEvent.button = GetMouseButtonIndex(mouseButtonState);
        window->dispatcher->PostEvent(e);

        mouseButtonState.reset();
    }
    else if (deviceType == PointerDeviceType::Touch)
    {

    }
}

void WindowWinUWPCxxBridge::OnPointerMoved(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg)
{
    using namespace ::Windows::UI::Input;
    using namespace ::Windows::Devices::Input;

    PointerPoint^ pointerPoint = arg->GetCurrentPoint(nullptr);
    PointerPointProperties^ prop = pointerPoint->Properties;
    PointerDeviceType deviceType = pointerPoint->PointerDevice->PointerDeviceType;

    DispatcherEvent e;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window->window;

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
                window->dispatcher->PostEvent(e);
            }
        }

        e.type = DispatcherEvent::MOUSE_MOVE;
        e.mmoveEvent.x = pointerPoint->Position.X;
        e.mmoveEvent.y = pointerPoint->Position.Y;
        window->dispatcher->PostEvent(e);

        mouseButtonState = state;
    }
    else if (deviceType == PointerDeviceType::Touch)
    {

    }
}

void WindowWinUWPCxxBridge::OnPointerWheelChanged(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg)
{
    using namespace ::Windows::UI::Input;

    PointerPoint^ pointerPoint = arg->GetCurrentPoint(nullptr);

    DispatcherEvent e;
    e.type = DispatcherEvent::MOUSE_WHEEL;
    e.timestamp = SystemTimer::Instance()->FrameStampTimeMS();
    e.window = window->window;
    e.mwheelEvent.x = pointerPoint->Position.X;
    e.mwheelEvent.y = pointerPoint->Position.Y;
    e.mwheelEvent.delta = pointerPoint->Properties->MouseWheelDelta;
    window->dispatcher->PostEvent(e);
}

uint32 WindowWinUWPCxxBridge::GetMouseButtonIndex(::Windows::UI::Input::PointerPointProperties^ props)
{
    if (props->IsLeftButtonPressed) return 1;
    if (props->IsRightButtonPressed) return 2;
    if (props->IsMiddleButtonPressed) return 3;
    if (props->IsXButton1Pressed) return 4;
    if (props->IsXButton2Pressed) return 5;
    return 0;
}

uint32 WindowWinUWPCxxBridge::GetMouseButtonIndex(std::bitset<5> state)
{
    for (size_t i = 0, n = state.size(); i < n; ++i)
    {
        if (state[i])
            return static_cast<uint32>(i + 1);
    }
    return 0;
}

std::bitset<5> WindowWinUWPCxxBridge::FillMouseButtonState(::Windows::UI::Input::PointerPointProperties^ props)
{
    std::bitset<5> state;
    if (props->IsLeftButtonPressed) state.set(0);
    if (props->IsRightButtonPressed) state.set(1);
    if (props->IsMiddleButtonPressed) state.set(2);
    if (props->IsXButton1Pressed) state.set(3);
    if (props->IsXButton2Pressed) state.set(4);
    return state;
}

void WindowWinUWPCxxBridge::InstallEventHandlers()
{
    using namespace ::Platform;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::UI::Xaml;
    using namespace ::Windows::UI::Xaml::Input;
    using namespace ::Windows::UI::Xaml::Controls;

    xamlWindow->Activated += ref new WindowActivatedEventHandler(this, &WindowWinUWPCxxBridge::OnActivated);
    xamlWindow->VisibilityChanged += ref new WindowVisibilityChangedEventHandler(this, &WindowWinUWPCxxBridge::OnVisibilityChanged);

    xamlWindow->CoreWindow->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &WindowWinUWPCxxBridge::OnCharacterReceived);
    xamlWindow->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher^, AcceleratorKeyEventArgs^>(this, &WindowWinUWPCxxBridge::OnAcceleratorKeyActivated);

    xamlSwapChainPanel->SizeChanged += ref new SizeChangedEventHandler(this, &WindowWinUWPCxxBridge::OnSizeChanged);
    xamlSwapChainPanel->CompositionScaleChanged += ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &WindowWinUWPCxxBridge::OnCompositionScaleChanged);

    xamlSwapChainPanel->PointerPressed += ref new PointerEventHandler(this, &WindowWinUWPCxxBridge::OnPointerPressed);
    xamlSwapChainPanel->PointerReleased += ref new PointerEventHandler(this, &WindowWinUWPCxxBridge::OnPointerReleased);
    xamlSwapChainPanel->PointerMoved += ref new PointerEventHandler(this, &WindowWinUWPCxxBridge::OnPointerMoved);
    xamlSwapChainPanel->PointerWheelChanged += ref new PointerEventHandler(this, &WindowWinUWPCxxBridge::OnPointerWheelChanged);
}

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
