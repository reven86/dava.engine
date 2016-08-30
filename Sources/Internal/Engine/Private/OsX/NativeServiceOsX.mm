#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Osx/NativeServiceOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/PlatformCoreOsX.h"

namespace DAVA
{
NativeService::NativeService(Private::PlatformCore* c)
    : core(c)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
