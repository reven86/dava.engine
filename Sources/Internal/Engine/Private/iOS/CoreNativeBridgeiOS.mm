#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/CoreNativeBridgeiOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/iOS/PlatformCoreiOS.h"
#include "Engine/Private/iOS/WindowBackendiOS.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

#include "Logger/Logger.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
namespace Private
{
CoreNativeBridgeiOS::CoreNativeBridgeiOS(PlatformCore* c)
    : core(c)
{
}

CoreNativeBridgeiOS::~CoreNativeBridgeiOS()
{
}

void CoreNativeBridgeiOS::Quit()
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
