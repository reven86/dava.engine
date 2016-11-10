#include "Engine/Private/Android/DeviceManagerImplAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/DeviceManager.h"
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

#endif // __DAVAENGINE_ANDROID__
