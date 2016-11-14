#include "Engine/UWP/NativeServiceUWP.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/UWP/PlatformCoreUWP.h"

namespace DAVA
{
NativeService::NativeService(Private::PlatformCore* c)
    : core(c)
{
}

void NativeService::RegisterXamlApplicationListener(XamlApplicationListener* listener)
{
    core->RegisterXamlApplicationListener(listener);
}

void NativeService::UnregisterXamlApplicationListener(XamlApplicationListener* listener)
{
    core->UnregisterXamlApplicationListener(listener);
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
