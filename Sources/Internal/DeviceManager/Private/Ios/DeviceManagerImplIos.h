#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "DeviceManager/DeviceManagerTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class DeviceManager;
namespace Private
{
struct DeviceManagerImpl final
{
    DeviceManagerImpl(DeviceManager* devManager, Private::MainDispatcher* dispatcher);

    void UpdateDisplayConfig();

    DeviceManager* deviceManager = nullptr;
    Private::MainDispatcher* mainDispatcher = nullptr;

    static float32 GetIPhoneMainScreenDpi();
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
