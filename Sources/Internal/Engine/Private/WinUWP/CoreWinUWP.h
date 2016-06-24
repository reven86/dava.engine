#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

#include "Concurrency/Mutex.h"

namespace DAVA
{
namespace Private
{
class CoreWinUWP final
{
public:
    CoreWinUWP(EngineBackend* e);
    ~CoreWinUWP();

    Vector<String> GetCommandLine(int argc, char* argv[]);

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

    bool gameThreadRunning = false;
    bool quitGameThread = false;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
