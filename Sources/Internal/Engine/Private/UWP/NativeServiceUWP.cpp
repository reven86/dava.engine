#include "Engine/Public/UWP/NativeServiceUWP.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/UWP/WindowNativeServiceUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/UWP/PlatformCoreUWP.h"

namespace DAVA
{
NativeService::NativeService(Private::PlatformCore* c)
    : core(c)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
