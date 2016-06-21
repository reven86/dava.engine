#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EngineFwd.h"

namespace DAVA
{
class Thread;
namespace Private
{
// clang-format off

class CoreWinUWP final
{
public:
    CoreWinUWP(EngineBackend* e);
    ~CoreWinUWP();

    Vector<String> GetCommandLine(int argc, char* argv[]);

    void Init();
    void Run();
    void Quit();

    WindowWinUWP* CreateNativeWindow(WindowBackend* w, float32 width, float32 height);
    void DestroyNativeWindow(WindowBackend* w);

    void OnApplicationLaunched();
    void OnNativeWindowCreated(::Windows::UI::Xaml::Window^ xamlWindow);

    void OnSuspending();
    void OnResuming();

private:
    void GameThread();

private:
    EngineBackend* engineBackend = nullptr;

    Thread* gameThread = nullptr;   // TODO: RefPtr
    bool quitGameThread = false;
};

// clang-format on

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
