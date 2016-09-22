#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EnginePrivateFwd.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
ref struct WindowNativeBridge sealed
{
    internal :
    WindowNativeBridge(WindowBackend* window);

    void* GetHandle() const;

    void BindToXamlWindow(::Windows::UI::Xaml::Window ^ xamlWnd);

    void AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
    void UnfocusXamlControl();

    void TriggerPlatformEvents();

    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();

    void SetMouseMode(eMouseMode newMode);
    eMouseMode GetMouseMode() const;

private:
    void SetMouseVisibility(bool visible);
    void SetMouseCaptured(bool capture);
    bool DeferredMouseMode(const MainDispatcherEvent& e);

    void OnTriggerPlatformEvents();

    void OnActivated(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::WindowActivatedEventArgs ^ arg);
    void OnVisibilityChanged(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::VisibilityChangedEventArgs ^ arg);

    void OnCharacterReceived(::Windows::UI::Core::CoreWindow ^ coreWindow, ::Windows::UI::Core::CharacterReceivedEventArgs ^ arg);
    void OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher ^ dispatcher, ::Windows::UI::Core::AcceleratorKeyEventArgs ^ arg);

    void OnSizeChanged(::Platform::Object ^ sender, ::Windows::UI::Xaml::SizeChangedEventArgs ^ arg);
    void OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel ^ panel, ::Platform::Object ^ obj);

    void OnPointerPressed(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg);
    void OnPointerReleased(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg);
    void OnPointerMoved(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg);
    void OnPointerWheelChanged(::Platform::Object ^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ arg);
    void OnMouseMoved(Windows::Devices::Input::MouseDevice ^ mouseDevice, Windows::Devices::Input::MouseEventArgs ^ args);

    static uint32 GetMouseButtonIndex(::Windows::UI::Input::PointerPointProperties ^ props);
    static uint32 GetMouseButtonIndex(std::bitset<5> state);
    static std::bitset<5> FillMouseButtonState(::Windows::UI::Input::PointerPointProperties ^ props);

    void CreateBaseXamlUI();
    void InstallEventHandlers();
    void UninstallEventHandlers();

private:
    WindowBackend* uwpWindow = nullptr;

    ::Windows::UI::Xaml::Window ^ xamlWindow = nullptr;
    ::Windows::UI::Xaml::Controls::SwapChainPanel ^ xamlSwapChainPanel = nullptr;
    ::Windows::UI::Xaml::Controls::Canvas ^ xamlCanvas = nullptr;
    ::Windows::UI::Xaml::Controls::Button ^ xamlControlThatStealsFocus = nullptr;

    std::bitset<5> mouseButtonState;

    // Tokens to unsubscribe from event handlers
    ::Windows::Foundation::EventRegistrationToken tokenActivated;
    ::Windows::Foundation::EventRegistrationToken tokenVisibilityChanged;
    ::Windows::Foundation::EventRegistrationToken tokenCharacterReceived;
    ::Windows::Foundation::EventRegistrationToken tokenAcceleratorKeyActivated;
    ::Windows::Foundation::EventRegistrationToken tokenSizeChanged;
    ::Windows::Foundation::EventRegistrationToken tokenCompositionScaleChanged;
    ::Windows::Foundation::EventRegistrationToken tokenPointerPressed;
    ::Windows::Foundation::EventRegistrationToken tokenPointerReleased;
    ::Windows::Foundation::EventRegistrationToken tokenPointerMoved;
    ::Windows::Foundation::EventRegistrationToken tokenPointerWheelChanged;
    ::Windows::Foundation::EventRegistrationToken tokenMouseMoved;

    static ::Platform::String ^ xamlWorkaroundWebViewProblems;
    static ::Platform::String ^ xamlWorkaroundTextBoxProblems;

    bool mouseCaptured = false;
    bool mouseVisibled = true;
    bool deferredMouseMode = false;
    eMouseMode mouseMode = eMouseMode::OFF;
    eMouseMode nativeMouseMode = eMouseMode::OFF;
    bool hasFocus = false;
    bool focusChanged = false;
    uint32 skipMouseMoveEvents = 0;
    const uint32 SKIP_N_MOUSE_MOVE_EVENTS = 4;
};

inline void* WindowNativeBridge::GetHandle() const
{
    return reinterpret_cast<void*>(xamlSwapChainPanel);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
