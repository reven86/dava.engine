#include "Engine/Private/OsX/PlatformCoreOsX.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Engine.h"
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

PlatformCore::~PlatformCore()
{
    Engine::Instance()->windowCreated.Disconnect(this);
    Engine::Instance()->windowDestroyed.Disconnect(this);
}

void PlatformCore::Init()
{
    // Subscribe to window creation & destroying, used for handling screen timeout option
    Engine::Instance()->windowCreated.Connect(this, &PlatformCore::OnWindowCreated);
    Engine::Instance()->windowDestroyed.Connect(this, &PlatformCore::OnWindowDestroyed);

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
    screenTimeoutEnabled = enabled;
    UpdateIOPMAssertion();
}

void PlatformCore::OnWindowCreated(Window* window)
{
    window->visibilityChanged.Connect(this, &PlatformCore::OnWindowVisibilityChanged);
}

void PlatformCore::OnWindowDestroyed(Window* window)
{
    window->visibilityChanged.Disconnect(this);
}

void PlatformCore::OnWindowVisibilityChanged(Window* window, bool visible)
{
    UpdateIOPMAssertion();
}

void PlatformCore::UpdateIOPMAssertion()
{
    bool useCustomAssertion = false;

    if (!screenTimeoutEnabled)
    {
        // User requested disabling screen timeout
        // Do that only if at least one window is visible
        for (Window* w : engineBackend->GetWindows())
        {
            if (w->IsVisible())
            {
                useCustomAssertion = true;
                break;
            }
        }
    }

    if (useCustomAssertion == false)
    {
        // Free the previous one, if any
        if (screenTimeoutAssertionId != kIOPMNullAssertionID)
        {
            const IOReturn releaseResult = IOPMAssertionRelease(screenTimeoutAssertionId);
            DVASSERT(releaseResult == kIOReturnSuccess);

            screenTimeoutAssertionId = kIOPMNullAssertionID;
        }
    }
    else if (screenTimeoutAssertionId == kIOPMNullAssertionID)
    {
        // Otherwise, create custom assertion if it hasn't been yet

        const IOReturn createResult = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                                  kIOPMAssertionLevelOn,
                                                                  CFSTR("Dava Engine application is running"),
                                                                  &screenTimeoutAssertionId);
        DVASSERT(createResult == kIOReturnSuccess);
    }
}

int32 PlatformCore::OnFrame()
{
    return engineBackend->OnFrame();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
