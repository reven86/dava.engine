#include "Engine/Private/OsX/PlatformCoreOsX.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Window.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
namespace Private
{
PlatformCore::PlatformCore(EngineBackend* engineBackend)
    : engineBackend(engineBackend)
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
    bridge->Quit();
}

void PlatformCore::SetScreenTimeoutEnabled(bool enabled)
{
    const bool timeoutEnabledNow = (screenTimeoutAssertionId == kIOPMNullAssertionID);
    if (timeoutEnabledNow == enabled)
    {
        return;
    }

    IOReturn result;
    if (enabled)
    {
        result = IOPMAssertionRelease(screenTimeoutAssertionId);
        screenTimeoutAssertionId = kIOPMNullAssertionID;
    }
    else
    {
        result = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                kIOPMAssertionLevelOn,
                                                CFSTR("Dava Engine application is running"),
                                                &screenTimeoutAssertionId);
    }

    DVASSERT(result == kIOReturnSuccess, Format("IOPMAssertion api failed in PlatformCore::SetScreenTimeoutEnabled(%s)", enabled ? "true" : "false").c_str());
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
