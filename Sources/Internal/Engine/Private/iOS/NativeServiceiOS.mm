#if defined(__DAVAENGINE_COREV2__)

#include "Engine/iOS/NativeServiceiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

namespace DAVA
{
NativeService::NativeService(Private::CoreNativeBridge* bridge_)
    : bridge(bridge_)
{
}

void NativeService::RegisterUIApplicationDelegateListener(UIApplicationDelegateListener* listener)
{
    bridge->RegisterUIApplicationDelegateListener(listener);
}

void NativeService::UnregisterUIApplicationDelegateListener(UIApplicationDelegateListener* listener)
{
    bridge->UnregisterUIApplicationDelegateListener(listener);
}

} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
