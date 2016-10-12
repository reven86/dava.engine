#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/EngineTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
ref struct WindowNativeBridge sealed
{
    internal :
    WindowNativeBridge(WindowBackend* windowBackend);

    void* GetHandle() const;

    void BindToXamlWindow(::Windows::UI::Xaml::Window ^ xamlWnd);

    void AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
    void UnfocusXamlControl();

    void TriggerPlatformEvents();

    void ResizeWindow(float32 width, float32 height);
    void CloseWindow();
    void SetTitle(const char8* title);

private:
    // Shortcut for eMouseButtons::COUNT
    static const size_t MOUSE_BUTTON_COUNT = static_cast<size_t>(eMouseButtons::COUNT);

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

    eModifierKeys GetModifierKeys() const;

    static eMouseButtons GetMouseButtonIndex(::Windows::UI::Input::PointerPointProperties ^ props);
    static eMouseButtons GetMouseButtonIndex(std::bitset<MOUSE_BUTTON_COUNT> state);
    static std::bitset<MOUSE_BUTTON_COUNT> FillMouseButtonState(::Windows::UI::Input::PointerPointProperties ^ props);

    void CreateBaseXamlUI();
    void InstallEventHandlers();
    void UninstallEventHandlers();

private:
    WindowBackend* windowBackend = nullptr;
    Window* window = nullptr;
    MainDispatcher* mainDispatcher = nullptr;

    ::Windows::UI::Xaml::Window ^ xamlWindow = nullptr;
    ::Windows::UI::Xaml::Controls::SwapChainPanel ^ xamlSwapChainPanel = nullptr;
    ::Windows::UI::Xaml::Controls::Canvas ^ xamlCanvas = nullptr;
    ::Windows::UI::Xaml::Controls::Button ^ xamlControlThatStealsFocus = nullptr;

    std::bitset<MOUSE_BUTTON_COUNT> mouseButtonState;

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

    static ::Platform::String ^ xamlWorkaroundWebViewProblems;
    static ::Platform::String ^ xamlWorkaroundTextBoxProblems;
};

inline void* WindowNativeBridge::GetHandle() const
{
    return reinterpret_cast<void*>(xamlSwapChainPanel);
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
