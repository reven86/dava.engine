#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Private/EnginePrivateFwd.h"

// Implementation of NSApplicationDelegate
// Forwards all necessary methods to CoreNativeBridgeOsX
@interface OsXAppDelegate : NSObject<NSApplicationDelegate>
{
    DAVA::Private::CoreNativeBridgeOsX* bridge;
}

- (id)init:(DAVA::Private::CoreNativeBridgeOsX*)objcBridge;

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
