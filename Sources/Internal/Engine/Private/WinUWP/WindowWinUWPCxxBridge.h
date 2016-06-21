#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

#include "Functional/Function.h"

namespace DAVA
{
namespace Private
{
// clang-format off

ref struct WindowWinUWPCxxBridge sealed
{
internal:
    WindowWinUWPCxxBridge(WindowWinUWP* window_);

    void RunAsyncOnUIThread(const Function<void()>& task);

    void BindToXamlWindow(::Windows::UI::Xaml::Window^ xamlWindow_);

    void OnActivated(::Platform::Object^ sender, ::Windows::UI::Core::WindowActivatedEventArgs^ arg);
    void OnVisibilityChanged(::Platform::Object^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ arg);

    void OnCharacterReceived(::Windows::UI::Core::CoreWindow^ coreWindow, ::Windows::UI::Core::CharacterReceivedEventArgs^ arg);
    void OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher^ dispatcher, ::Windows::UI::Core::AcceleratorKeyEventArgs^ arg);

    void OnSizeChanged(::Platform::Object^ sender, ::Windows::UI::Xaml::SizeChangedEventArgs^ arg);
    void OnCompositionScaleChanged(::Windows::UI::Xaml::Controls::SwapChainPanel^ panel, ::Platform::Object^ obj);

    void OnPointerPressed(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg);
    void OnPointerReleased(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg);
    void OnPointerMoved(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg);
    void OnPointerWheelChanged(::Platform::Object^ sender, ::Windows::UI::Xaml::Input::PointerRoutedEventArgs^ arg);

    static uint32 GetMouseButtonIndex(::Windows::UI::Input::PointerPointProperties^ props);
    static uint32 GetMouseButtonIndex(std::bitset<5> state);
    static std::bitset<5> FillMouseButtonState(::Windows::UI::Input::PointerPointProperties^ props);

    void InstallEventHandlers();

    WindowWinUWP* window = nullptr;

    ::Windows::UI::Xaml::Window^ xamlWindow = nullptr;
    ::Windows::UI::Xaml::Controls::SwapChainPanel^ xamlSwapChainPanel = nullptr;
    ::Windows::UI::Xaml::Controls::Canvas^ xamlCanvas = nullptr;

    std::bitset<5> mouseButtonState;
};

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
