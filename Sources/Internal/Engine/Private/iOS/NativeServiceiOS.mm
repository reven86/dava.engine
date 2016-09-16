#if defined(__DAVAENGINE_COREV2__)

#include "Engine/iOS/NativeServiceiOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/PlatformCoreiOS.h"

namespace DAVA
{
NativeService::NativeService(Private::PlatformCore* c)
    : core(c)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
