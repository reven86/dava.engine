#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
namespace Private
{
class CoreWinUWP final
{
public:
    CoreWinUWP(EngineBackend* e);
    ~CoreWinUWP();

    NativeService* GetNativeService() const;

    void Init();
    void Run();
    void Quit();

    // Forwarded methods from UWPApplication
    void OnLaunched();
    void OnActivated();
    void OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow);
    void OnSuspending();
    void OnResuming();

private:
    void GameThread();

private:
    EngineBackend* engineBackend = nullptr;
    std::unique_ptr<NativeService> nativeService;

    bool gameThreadRunning = false;
    bool quitGameThread = false;
};

inline NativeService* CoreWinUWP::GetNativeService() const
{
    return nativeService.get();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
