#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineFwd.h"
#include "Engine/Private/OsX/OsXFwd.h"

#import <Foundation/NSGeometry.h>

@class NSView;
@class NSBitmapImageRep;

namespace DAVA
{
namespace Private
{
class WindowInteropService final
{
public:
    WindowInteropService(WindowOsX* w, WindowOsXObjcBridge* objcBridge);

    void AddNSView(NSView* nsview);
    void RemoveNSView(NSView* nsview);

    NSRect ConvertRectFromBacking(const NSRect& rect);
    NSBitmapImageRep* BitmapImageRepForCachingDisplayInRect(const NSRect& rect);

    NSView* GetNSView();

private:
    WindowOsX* nativeWindow = nullptr;
    WindowOsXObjcBridge* bridge = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
