#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Private/OsX/OsXFwd.h"

// Implementation of NSApplicationDelegate
// Forwards all necessary methods to CoreOsXObjcBridge
@interface OsXAppDelegate : NSObject<NSApplicationDelegate>
{
    DAVA::Private::CoreOsXObjcBridge* bridge;
}

- (id)init:(DAVA::Private::CoreOsXObjcBridge*)objcBridge;

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
