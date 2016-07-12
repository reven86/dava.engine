#include "Engine/Public/WinUWP/NativeServiceWinUWP.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/WinUWP/WindowNativeServiceWinUWP.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/WinUWP/CoreWinUWP.h"

namespace DAVA
{
NativeService::NativeService(Private::CoreWinUWP* c)
    : core(c)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
