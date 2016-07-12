#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/OsX/WindowNativeServiceOsX.h"

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/WindowOsX.h"
#include "Engine/Private/OsX/WindowOsXObjcBridge.h"

#import "Engine/Private/OsX/OpenGLViewOsX.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowOsXObjcBridge* objcBridge)
    : bridge(objcBridge)
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
