#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

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
    void OnLaunchedOrActivated(::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
    void OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow);
    void OnSuspending();
    void OnResuming();
    void OnUnhandledException(::Windows::UI::Xaml::UnhandledExceptionEventArgs ^ arg);
    void OnBackPressed();
    void OnGamepadAdded(::Windows::Gaming::Input::Gamepad ^ gamepad);
    void OnGamepadRemoved(::Windows::Gaming::Input::Gamepad ^ gamepad);
    void OnDpiChanged();

    static bool IsPhoneContractPresent();

private:
    void GameThread();

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* dispatcher = nullptr;

    bool gameThreadRunning = false;
    bool quitGameThread = false;

    static bool isPhoneContractPresent;
};

inline bool PlatformCore::IsPhoneContractPresent()
{
    return isPhoneContractPresent;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
