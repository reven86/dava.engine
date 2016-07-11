#include "Engine/Private/OsX/WindowInteropService.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/WindowOsX.h"
#include "Engine/Private/OsX/WindowOsXObjcBridge.h"

#import "Engine/Private/OsX/OpenGLViewOsX.h"

namespace DAVA
{
namespace Private
{
WindowInteropService::WindowInteropService(WindowOsX* w, WindowOsXObjcBridge* objcBridge)
    : nativeWindow(w)
    , bridge(objcBridge)
{
}

void WindowInteropService::AddNSView(NSView* nsview)
{
    [bridge->openGLView addSubview:nsview];
}

void WindowInteropService::RemoveNSView(NSView* nsview)
{
    [nsview removeFromSuperview];
}

NSRect WindowInteropService::ConvertRectFromBacking(const NSRect& rect)
{
    return [bridge->openGLView convertRectFromBacking:rect];
}

NSBitmapImageRep* WindowInteropService::BitmapImageRepForCachingDisplayInRect(const NSRect& rect)
{
    return [bridge->openGLView bitmapImageRepForCachingDisplayInRect:rect];
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
