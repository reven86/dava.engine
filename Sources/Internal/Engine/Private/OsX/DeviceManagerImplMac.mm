#include "Engine/Private/OsX/DeviceManagerImplMac.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_MACOS__)

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

#endif // __DAVAENGINE_MACOS__
