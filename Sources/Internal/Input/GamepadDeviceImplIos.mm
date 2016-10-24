#include "Input/GamepadDeviceImplIos.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/GamepadDevice.h"

namespace DAVA
{
namespace Private
{
GamepadDeviceImpl::GamepadDeviceImpl(GamepadDevice* gamepadDevice)
    : gamepadDevice(gamepadDevice)
{
}

void GamepadDeviceImpl::Update()
{
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 id)
{
    return false;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 id)
{
    return false;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
