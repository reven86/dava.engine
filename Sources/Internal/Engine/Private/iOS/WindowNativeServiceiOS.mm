#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Public/iOS/WindowNativeServiceiOS.h"

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/WindowBackendiOS.h"
#include "Engine/Private/iOS/WindowNativeBridgeiOS.h"

namespace DAVA
{
WindowNativeService::WindowNativeService(Private::WindowNativeBridgeiOS* nativeBridge)
    : bridge(nativeBridge)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
