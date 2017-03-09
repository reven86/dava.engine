#include "Engine/Private/iOS/PlatformCoreiOS.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Window.h"
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
    int exitCode = engineBackend->GetExitCode();
    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();

    std::exit(exitCode);
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
