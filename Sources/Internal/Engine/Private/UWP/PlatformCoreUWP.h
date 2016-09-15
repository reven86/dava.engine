#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
namespace Private
{
class PlatformCore final
{
public:
    PlatformCore(EngineBackend* e);
    ~PlatformCore();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void Quit(bool triggeredBySystem);

    // Forwarded methods from UWPApplication
    void OnLaunched();
    void OnActivated();
    void OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow);
    void OnSuspending();
    void OnResuming();
    void OnUnhandledException(::Windows::UI::Xaml::UnhandledExceptionEventArgs ^ arg);

private:
    void GameThread();

private:
    EngineBackend* engineBackend = nullptr;
    MainDispatcher* dispatcher = nullptr;
    std::unique_ptr<NativeService> nativeService;

    bool gameThreadRunning = false;
    bool quitGameThread = false;
};

inline NativeService* PlatformCore::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
