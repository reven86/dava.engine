#if defined(__DAVAENGINE_COREV2__)

#include "Engine/OsX/WindowNativeServiceOsX.h"

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#import "Engine/Private/OsX/Window/RenderViewOsX.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowNativeBridge* nativeBridge)
    : bridge(nativeBridge)
{
}

void WindowNativeService::AddNSView(NSView* nsview)
{
    [bridge->renderView addSubview:nsview];
}

void WindowNativeService::RemoveNSView(NSView* nsview)
{
    [nsview removeFromSuperview];
}
void WindowNativeService::DoWindowDeminiaturize()
{
    [bridge->nswindow deminiaturize:bridge->windowDelegate];
}

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
