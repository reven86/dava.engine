#include "DeviceManager/Private/Ios/DeviceManagerImplIos.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "DeviceManager/DeviceManager.h"
#include "Engine/Private/Dispatcher/MainDispatcher.h"

namespace DAVA
{
namespace Private
{
DeviceManagerImpl::DeviceManagerImpl(DeviceManager* devManager, Private::MainDispatcher* dispatcher)
    : deviceManager(devManager)
    , mainDispatcher(dispatcher)
{
}

void DeviceManagerImpl::UpdateDisplayConfig()
{
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
