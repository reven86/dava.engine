#include "Engine/Public/Win32/NativeServiceWin32.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/Osx/WindowNativeServiceOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/CoreOsX.h"

namespace DAVA
{
NativeService::NativeService(Private::CoreOsX* c)
    : core(c)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
