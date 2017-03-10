#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Concurrency/Mutex.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* engineBackend_);
    ~PlatformCore();

    void Init();
    void Run();
    void PrepareToQuit();
    void Quit();

    // Forwarded methods from UWPApplication
    void OnLaunchedOrActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs ^ args);
    void OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow);
    void OnSuspending();
    void OnResuming();
    void OnUnhandledException(::Windows::UI::Xaml::UnhandledExceptionEventArgs ^ arg);
    void OnBackPressed();
    void OnGamepadAdded(::Windows::Gaming::Input::Gamepad ^ gamepad);
    void OnGamepadRemoved(::Windows::Gaming::Input::Gamepad ^ gamepad);
    void OnDpiChanged();

    static bool IsPhoneContractPresent();
    static void EnableHighResolutionTimer(bool enable);
    static ::Windows::UI::Core::CoreDispatcher ^ GetCoreDispatcher();

    void RegisterXamlApplicationListener(PlatformApi::Win10::XamlApplicationListener* listener);
    void UnregisterXamlApplicationListener(PlatformApi::Win10::XamlApplicationListener* listener);

private:
    void GameThread();

    enum eNotificationType
    {
        ON_LAUNCHED,
        ON_ACTIVATED,
        ON_SUSPENDING,
    };
    void NotifyListeners(eNotificationType type, ::Platform::Object ^ arg1);

    EngineBackend* engineBackend = nullptr;
    MainDispatcher* dispatcher = nullptr;

    bool gameThreadRunning = false;
    bool quitGameThread = false;
    bool appPrelaunched = false;

    Mutex listenersMutex;
    List<PlatformApi::Win10::XamlApplicationListener*> xamlApplicationListeners;
    ::Windows::ApplicationModel::Activation::IActivatedEventArgs ^ savedActivatedEventArgs = nullptr;

    static bool isPhoneContractPresent;
    static ::Windows::UI::Core::CoreDispatcher ^ coreDispatcher;
};

inline bool PlatformCore::IsPhoneContractPresent()
{
    return isPhoneContractPresent;
}

inline ::Windows::UI::Core::CoreDispatcher ^ PlatformCore::GetCoreDispatcher()
{
    return coreDispatcher;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
