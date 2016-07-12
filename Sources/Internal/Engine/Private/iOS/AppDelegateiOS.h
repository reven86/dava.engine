#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#import <UIKit/UIApplication.h>

#include "Engine/Private/EnginePrivateFwd.h"

// Implementation of UIApplicationDelegate
// Forwards all necessary methods to CoreNativeBridgeiOS
@interface AppDelegateiOS : NSObject<UIApplicationDelegate>
{
    DAVA::Private::CoreNativeBridgeiOS* bridge;
}

- (id)init:(DAVA::Private::CoreNativeBridgeiOS*)nativeBridge;

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
