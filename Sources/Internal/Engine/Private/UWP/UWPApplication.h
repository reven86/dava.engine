#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

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

public:
    virtual ~UWPApplication();

protected:
    // ::Windows::UI::Xaml::Application overriden methods
    void OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;
    void OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args) override;
    void OnWindowCreated(::Windows::UI::Xaml::WindowCreatedEventArgs^ args) override;

private:
    void OnSuspending(::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ arg);
    void OnResuming(::Platform::Object^ sender, ::Platform::Object^ arg);

private:
    EngineBackend* engineBackend = nullptr;
};

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
