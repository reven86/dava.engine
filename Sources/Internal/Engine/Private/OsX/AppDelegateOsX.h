#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#import <AppKit/NSApplication.h>

#include "Engine/Private/EnginePrivateFwd.h"

// Implementation of NSApplicationDelegate
// Forwards all necessary methods to CoreNativeBridge
@interface AppDelegate : NSObject<NSApplicationDelegate>
{
    DAVA::Private::CoreNativeBridge* bridge;
}

- (id)initWithBridge:(DAVA::Private::CoreNativeBridge*)nativeBridge;

@end

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
