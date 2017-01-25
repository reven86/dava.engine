#include "Engine/Private/UWP/UWPApplication.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/UWP/PlatformCoreUWP.h"

// clang-format off

namespace DAVA
{
namespace Private
{

int StartUWPApplication(Vector<String> cmdargs)
{
    using namespace ::Windows::UI::Xaml;
    auto appStartCallback = ref new ApplicationInitializationCallback([cmdargs=std::move(cmdargs)](ApplicationInitializationCallbackParams^) {
        ref new DAVA::Private::UWPApplication(std::move(cmdargs));
    });
    Application::Start(appStartCallback);
    return 0;
}

UWPApplication::UWPApplication(Vector<String> cmdargs)
    : commandArgs(std::move(cmdargs))
{
}

void UWPApplication::OnLaunched(::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
    OnLaunchedOrActivated(args);
}

void UWPApplication::OnActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
{
    OnLaunchedOrActivated(args);
}

void UWPApplication::OnLaunchedOrActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
{
    using ::Windows::ApplicationModel::Activation::ApplicationExecutionState;

    ApplicationExecutionState prevExecState = args->PreviousExecutionState;
    if (prevExecState != ApplicationExecutionState::Running && prevExecState != ApplicationExecutionState::Suspended)
    {
        // Install event handlers only if application is not running
        InstallEventHandlers();
    }

    core->OnLaunchedOrActivated(args);
}

void UWPApplication::OnWindowCreated(::Windows::UI::Xaml::WindowCreatedEventArgs^ args)
{
    if (engineBackend == nullptr)
    {
        // Create EngineBackend when application has entered thread where Universal Applications live (so called UI-thread).
        // Reason: Win10 platform implementation can access WinRT API when initializing EngineBackend (DeviceManager, etc).
        engineBackend.reset(new EngineBackend(commandArgs));
        core = engineBackend->GetPlatformCore();
        commandArgs.clear();
    }
    core->OnWindowCreated(args->Window);
}

void UWPApplication::OnSuspending(::Platform::Object^ /*sender*/, ::Windows::ApplicationModel::SuspendingEventArgs^ /*arg*/)
{
    core->OnSuspending();
}

void UWPApplication::OnResuming(::Platform::Object^ /*sender*/, ::Platform::Object^ /*arg*/)
{
    core->OnResuming();
}

void UWPApplication::OnUnhandledException(::Platform::Object^ /*sender*/, ::Windows::UI::Xaml::UnhandledExceptionEventArgs^ arg)
{
    core->OnUnhandledException(arg);
}

void UWPApplication::OnBackPressed(::Platform::Object^ /*sender*/, ::Windows::Phone::UI::Input::BackPressedEventArgs^ args)
{
    core->OnBackPressed();
    args->Handled = true;
}

void UWPApplication::OnBackRequested(::Platform::Object^ /*sender*/, ::Windows::UI::Core::BackRequestedEventArgs^ args)
{
    core->OnBackPressed();
    args->Handled = true;
}

void UWPApplication::OnGamepadAdded(::Platform::Object^ sender, ::Windows::Gaming::Input::Gamepad^ gamepad)
{
    core->OnGamepadAdded(gamepad);
}

void UWPApplication::OnGamepadRemoved(::Platform::Object^ sender, ::Windows::Gaming::Input::Gamepad^ gamepad)
{
    core->OnGamepadRemoved(gamepad);
}

void UWPApplication::OnDpiChanged(::Windows::Graphics::Display::DisplayInformation^ sender, ::Platform::Object^ args)
{
    core->OnDpiChanged();
}

void UWPApplication::InstallEventHandlers()
{
    using namespace ::Platform;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Xaml;
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::Gaming::Input;
    using namespace ::Windows::Phone::UI::Input;
    using namespace ::Windows::Graphics::Display;

    Suspending += ref new SuspendingEventHandler(this, &UWPApplication::OnSuspending);
    Resuming += ref new EventHandler<Object^>(this, &UWPApplication::OnResuming);
    UnhandledException += ref new UnhandledExceptionEventHandler(this, &UWPApplication::OnUnhandledException);

    SystemNavigationManager::GetForCurrentView()->BackRequested += ref new EventHandler<BackRequestedEventArgs^>(this, &UWPApplication::OnBackRequested);
    if (PlatformCore::IsPhoneContractPresent())
    {
        HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &UWPApplication::OnBackPressed);
    }

    Gamepad::GamepadAdded += ref new EventHandler<Gamepad^>(this, &UWPApplication::OnGamepadAdded);
    Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^>(this, &UWPApplication::OnGamepadRemoved);

    DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
    displayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWPApplication::OnDpiChanged);
}

} // namespace Private
} // namespace DAVA

// clang-format on

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
