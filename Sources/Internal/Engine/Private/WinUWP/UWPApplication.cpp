#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/WinUWP/UWPApplication.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/WindowBackend.h"
#include "Engine/Private/WinUWP/CoreWinUWP.h"

#include "Functional/Function.h"
#include "Concurrency/Thread.h"

// clang-format off

namespace DAVA
{
namespace Private
{

void StartUWPApplication()
{
    using namespace ::Windows::UI::Xaml;
    auto appStartCallback = ref new ApplicationInitializationCallback([](ApplicationInitializationCallbackParams^) {
        ref new DAVA::Private::UWPApplication();
    });
    Application::Start(appStartCallback);
}

UWPApplication::UWPApplication()
    : engineBackend(new EngineBackend(0, nullptr))
{
    
}

UWPApplication::~UWPApplication()
{
    delete engineBackend;
}

void UWPApplication::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    using namespace ::Windows::UI::Xaml;

    Suspending += ref new SuspendingEventHandler(this, &UWPApplication::OnSuspending);

    engineBackend->platformCore->OnApplicationLaunched();
}

void UWPApplication::OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
{

}

void UWPApplication::OnWindowCreated(::Windows::UI::Xaml::WindowCreatedEventArgs^ args)
{
    engineBackend->platformCore->OnNativeWindowCreated(args->Window);
}

void UWPApplication::OnSuspending(::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ arg)
{
    engineBackend->platformCore->OnSuspending();
}

void UWPApplication::OnResuming(::Platform::Object^ sender, ::Platform::Object^ arg)
{
    engineBackend->platformCore->OnResuming();
}

} // namespace Private
} // namespace DAVA

// clang-format on

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
