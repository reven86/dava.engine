#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/OsX/WindowNativeServiceOsX.h"

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/Window/WindowBackendOsX.h"
#include "Engine/Private/OsX/Window/WindowNativeBridgeOsX.h"

#import "Engine/Private/OsX/Window/OpenGLViewOsX.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowNativeBridgeOsX* nativeBridge)
    : bridge(nativeBridge)
{
}

void WindowNativeService::AddNSView(NSView* nsview)
{
    [bridge->openGLView addSubview:nsview];
}

void WindowNativeService::RemoveNSView(NSView* nsview)
{
    [nsview removeFromSuperview];
}

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
