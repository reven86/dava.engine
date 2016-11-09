#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
// clang-format off

ref class UWPApplication sealed : public ::Windows::UI::Xaml::Application
{
internal:
    UWPApplication(const Vector<String>& cmdargs);

protected:
    // ::Windows::UI::Xaml::Application overriden methods
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;
    void OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args) override;
    void OnWindowCreated(::Windows::UI::Xaml::WindowCreatedEventArgs^ args) override;

private:
    void OnSuspending(::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ arg);
    void OnResuming(::Platform::Object^ sender, ::Platform::Object^ arg);
    void OnUnhandledException(::Platform::Object^ sender, ::Windows::UI::Xaml::UnhandledExceptionEventArgs^ arg);

    void OnBackPressed(::Platform::Object^ sender, ::Windows::Phone::UI::Input::BackPressedEventArgs^ args);
    void OnBackRequested(::Platform::Object^ sender, ::Windows::UI::Core::BackRequestedEventArgs^ args);

    void OnGamepadAdded(::Platform::Object^ sender, ::Windows::Gaming::Input::Gamepad^ gamepad);
    void OnGamepadRemoved(::Platform::Object^ sender, ::Windows::Gaming::Input::Gamepad^ gamepad);

    void InstallEventHandlers();

private:
    std::unique_ptr<EngineBackend> engineBackend;
    PlatformCore* core = nullptr;
};

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
