#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/PlatformCoreiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Window.h"
#include "Engine/iOS/NativeServiceiOS.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/Window/WindowBackendiOS.h"
#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(engineBackend)
    , dispatcher(engineBackend->GetDispatcher())
    , bridge(new CoreNativeBridge(this))
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
    engineBackend->InitializePrimaryWindow();
}

void PlatformCore::Run()
{
    bridge->Run();
}

void PlatformCore::PrepareToQuit()
{
    engineBackend->PostAppTerminate(true);
}

void PlatformCore::Quit()
{
    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();

    std::exit(engineBackend->GetExitCode());
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

WindowBackend* PlatformCore::GetWindowBackend(Window* window)
{
    return window->GetBackend();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
