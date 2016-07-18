#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/UWPApplication.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/UWP/PlatformCoreUWP.h"

// clang-format off

namespace DAVA
{
namespace Private
{

int StartUWPApplication(const Vector<String>& cmdargs)
{
    using namespace ::Windows::UI::Xaml;
    auto appStartCallback = ref new ApplicationInitializationCallback([cmdargs](ApplicationInitializationCallbackParams^) {
        ref new DAVA::Private::UWPApplication(cmdargs);
    });
    Application::Start(appStartCallback);
    return 0;
}

UWPApplication::UWPApplication(const Vector<String>& cmdargs)
    : engineBackend(new EngineBackend(cmdargs))
    , core(engineBackend->GetPlatformCore())
{
}

void UWPApplication::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    using namespace ::Platform;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Xaml;

    Suspending += ref new SuspendingEventHandler(this, &UWPApplication::OnSuspending);
    Resuming += ref new EventHandler<Object^>(this, &UWPApplication::OnResuming);
    UnhandledException += ref new UnhandledExceptionEventHandler(this, &UWPApplication::OnUnhandledException);

    core->OnLaunched();
}

void UWPApplication::OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
{
    core->OnActivated();
}

void UWPApplication::OnWindowCreated(::Windows::UI::Xaml::WindowCreatedEventArgs^ args)
{
    core->OnWindowCreated(args->Window);
}

void UWPApplication::OnSuspending(::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ arg)
{
    core->OnSuspending();
}

void UWPApplication::OnResuming(::Platform::Object^ sender, ::Platform::Object^ arg)
{
    core->OnResuming();
}

void UWPApplication::OnUnhandledException(::Platform::Object^ sender, ::Windows::UI::Xaml::UnhandledExceptionEventArgs^ arg)
{
    core->OnUnhandledException(arg);
}

} // namespace Private
} // namespace DAVA

// clang-format on

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
