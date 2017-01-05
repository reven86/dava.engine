#include "Base/Platform.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/PlatformCoreOsX.h"
#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#import <AppKit/NSView.h>

namespace DAVA
{
namespace PlatformApi
{
namespace Mac
{
void AddNSView(Window* targetWindow, NSView* nsview)
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(targetWindow);
    [wb->bridge->renderView addSubview:nsview];
}

void RemoveNSView(Window* targetWindow, NSView* nsview)
{
    [nsview removeFromSuperview];
}

void PrimaryWindowDeminiaturize()
{
    using namespace DAVA::Private;
    WindowBackend* wb = EngineBackend::GetWindowBackend(EngineBackend::Instance()->GetPrimaryWindow());
    [wb->bridge->nswindow deminiaturize:wb->bridge->windowDelegate];
    [wb->bridge->nswindow becomeKeyWindow];
}

void RegisterDVEApplicationListener(id<DVEApplicationListener> listener)
{
    using namespace DAVA::Private;
    PlatformCore* core = EngineBackend::Instance()->GetPlatformCore();
    core->bridge->RegisterDVEApplicationListener(listener);
}

void UnregisterDVEApplicationListener(id<DVEApplicationListener> listener)
{
    using namespace DAVA::Private;
    PlatformCore* core = EngineBackend::Instance()->GetPlatformCore();
    core->bridge->UnregisterDVEApplicationListener(listener);
}

} // namespace Mac
} // namespace PlatformApi
} // namespace DAVA

#endif // defined(__DAVAENGINE_MACOS__)
#endif // defined(__DAVAENGINE_COREV2__)
