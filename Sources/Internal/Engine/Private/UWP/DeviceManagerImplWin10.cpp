#include "Engine/Private/UWP/DeviceManagerImplWin10.h"

#if defined(__DAVAENGINE_WIN_UAP__)

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

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
