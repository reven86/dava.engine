#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/OsX/PlatformCoreOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Public/OsX/NativeServiceOsX.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/WindowOsX.h"
#include "Engine/Private/OsX/PlatformCorebridge.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* e)
    : engineBackend(e)
    , bridge(new PlatformCorebridge(this))
    , nativeService(new NativeService(this))
{
}

PlatformCore::~PlatformCore()
{
    delete bridge;
}

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    bridge->InitNSApplication();
    NSApplicationMain(0, nullptr);
}

void PlatformCore::Quit()
{
    bridge->Quit();
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

WindowOsX* PlatformCore::CreateNativeWindow(Window* w, float32 width, float32 height)
{
    WindowOsX* nativeWindow = new WindowOsX(engineBackend, w);
    if (!nativeWindow->Create(width, height))
    {
        delete nativeWindow;
        nativeWindow = nullptr;
    }
    return nativeWindow;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
