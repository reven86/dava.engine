#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/PlatformCoreOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Public/OsX/NativeServiceOsX.h"
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
}

void PlatformCore::Run()
{
    bridge->Run();
}

void PlatformCore::Quit()
{
    bridge->Quit();
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

WindowBackend* PlatformCore::CreateNativeWindow(Window* w, float32 width, float32 height)
{
    WindowBackend* wbackend = new WindowBackend(engineBackend, w);
    if (!wbackend->Create(width, height))
    {
        delete wbackend;
        wbackend = nullptr;
    }
    return wbackend;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
