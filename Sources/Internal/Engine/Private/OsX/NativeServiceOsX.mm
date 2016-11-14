#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Osx/NativeServiceOsX.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

#include "Engine/Private/OsX/CoreNativeBridgeOsX.h"

namespace DAVA
{
NativeService::NativeService(Private::CoreNativeBridge* bridge_)
    : bridge(bridge_)
{
}

void NativeService::RegisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener)
{
    bridge->RegisterNSApplicationDelegateListener(listener);
}

void NativeService::UnregisterNSApplicationDelegateListener(NSApplicationDelegateListener* listener)
{
    bridge->UnregisterNSApplicationDelegateListener(listener);
}

} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
