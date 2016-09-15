#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/PlatformCoreOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Window.h"
#include "Engine/OsX/NativeServiceOsX.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* e)
    : engineBackend(e)
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

void PlatformCore::Quit(bool /*triggeredBySystem*/)
{
    bridge->Quit();
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

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
