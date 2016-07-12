#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/EngineFwd.h"

#import <Foundation/NSGeometry.h>

@class NSView;
@class NSBitmapImageRep;

namespace DAVA
{
class WindowNativeService final
{
public:
    void AddNSView(NSView* nsview);
    void RemoveNSView(NSView* nsview);

    NSRect ConvertRectFromBacking(const NSRect& rect);
    NSBitmapImageRep* BitmapImageRepForCachingDisplayInRect(const NSRect& rect);

    NSView* GetNSView();

private:
    WindowNativeService(Private::WindowOsXObjcBridge* objcBridge);

private:
    Private::WindowOsXObjcBridge* bridge = nullptr;
};

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
