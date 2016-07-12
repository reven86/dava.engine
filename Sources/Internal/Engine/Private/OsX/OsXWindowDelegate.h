#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSWindow.h>

#include "Engine/Private/EngineFwd.h"

// Implementation of NSWindowDelegate
// Forwards all necessary methods to WindowOsXObjcBridge
@interface OsXWindowDelegate : NSObject<NSWindowDelegate>
{
    DAVA::Private::WindowOsXObjcBridge* bridge;
}

- (id)init:(DAVA::Private::WindowOsXObjcBridge*)objcBridge;

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
