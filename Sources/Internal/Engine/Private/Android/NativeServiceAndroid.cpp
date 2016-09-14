#include "Engine/Android/NativeServiceAndroid.h"

#if defined(__DAVAENGINE_COREV2__)

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/Android/PlatformCoreAndroid.h"

namespace DAVA
{
NativeService::NativeService(Private::PlatformCore* c)
    : core(c)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
