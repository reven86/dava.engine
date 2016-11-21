#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#import <AppKit/NSView.h>

namespace DAVA
{
namespace PlatformApi
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

} // namespace PlatformApi
} // namespace DAVA

#endif // defined(__DAVAENGINE_MACOS__)
#endif // defined(__DAVAENGINE_COREV2__)
