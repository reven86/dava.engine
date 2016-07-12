#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/AppDelegateiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

@implementation AppDelegateiOS

- (id)init:(DAVA::Private::CoreNativeBridgeiOS*)nativeBridge
{
    self = [super init];
    if (self != nullptr)
    {
        bridge = nativeBridge;
    }
    return self;
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
