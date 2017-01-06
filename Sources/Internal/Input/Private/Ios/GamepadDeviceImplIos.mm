#include "Input/Private/Ios/GamepadDeviceImplIos.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#import <GameController/GameController.h>

#include "Input/GamepadDevice.h"
#include "Time/SystemTimer.h"

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
    float32 readBuf[GamepadDevice::ELEMENT_COUNT];
    if ([controller extendedGamepad])
    {
        GCExtendedGamepad* gamepad = [controller extendedGamepad];
        ReadExtendedGamepadElements(gamepad, readBuf);
    }
    else if ([controller gamepad])
    {
        GCGamepad* gamepad = [controller gamepad];
        ReadGamepadElements(gamepad, readBuf);
    }

    int64 timestamp = SystemTimer::GetMs();
    for (size_t i = 0; i < GamepadDevice::ELEMENT_COUNT; ++i)
    {
        if (gamepadDevice->elementValues[i] != readBuf[i])
        {
            gamepadDevice->elementValues[i] = readBuf[i];
            gamepadDevice->elementTimestamps[i] = timestamp;
            gamepadDevice->elementChangedMask.set(i);
        }
    }
}

void GamepadDeviceImpl::ReadExtendedGamepadElements(GCExtendedGamepad* gamepad, float32 buf[])
{
    buf[static_cast<size_t>(eGamepadElements::A)] = static_cast<float32>(gamepad.buttonA.isPressed);
    buf[static_cast<size_t>(eGamepadElements::B)] = static_cast<float32>(gamepad.buttonB.isPressed);
    buf[static_cast<size_t>(eGamepadElements::X)] = static_cast<float32>(gamepad.buttonX.isPressed);
    buf[static_cast<size_t>(eGamepadElements::Y)] = static_cast<float32>(gamepad.buttonY.isPressed);

    buf[static_cast<size_t>(eGamepadElements::LEFT_SHOULDER)] = static_cast<float32>(gamepad.leftShoulder.isPressed);
    buf[static_cast<size_t>(eGamepadElements::RIGHT_SHOULDER)] = static_cast<float32>(gamepad.rightShoulder.isPressed);

    buf[static_cast<size_t>(eGamepadElements::LEFT_THUMBSTICK_X)] = gamepad.leftThumbstick.xAxis.value;
    buf[static_cast<size_t>(eGamepadElements::LEFT_THUMBSTICK_Y)] = gamepad.leftThumbstick.yAxis.value;
    buf[static_cast<size_t>(eGamepadElements::RIGHT_THUMBSTICK_X)] = gamepad.rightThumbstick.xAxis.value;
    buf[static_cast<size_t>(eGamepadElements::RIGHT_THUMBSTICK_Y)] = gamepad.rightThumbstick.yAxis.value;

    buf[static_cast<size_t>(eGamepadElements::LEFT_TRIGGER)] = gamepad.leftTrigger.value;
    buf[static_cast<size_t>(eGamepadElements::RIGHT_TRIGGER)] = gamepad.rightTrigger.value;

    float32 dpadX = 0.f;
    float32 dpadY = 0.f;
    if (gamepad.dpad.left.isPressed)
        dpadX = -1.f;
    else if (gamepad.dpad.right.isPressed)
        dpadX = 1.f;
    if (gamepad.dpad.down.isPressed)
        dpadY = -1.f;
    else if (gamepad.dpad.up.isPressed)
        dpadY = 1.f;

    buf[static_cast<size_t>(eGamepadElements::DPAD_X)] = dpadX;
    buf[static_cast<size_t>(eGamepadElements::DPAD_Y)] = dpadY;
}

void GamepadDeviceImpl::ReadGamepadElements(GCGamepad* gamepad, float32 buf[])
{
    buf[static_cast<size_t>(eGamepadElements::A)] = static_cast<float32>(gamepad.buttonA.isPressed);
    buf[static_cast<size_t>(eGamepadElements::B)] = static_cast<float32>(gamepad.buttonB.isPressed);
    buf[static_cast<size_t>(eGamepadElements::X)] = static_cast<float32>(gamepad.buttonX.isPressed);
    buf[static_cast<size_t>(eGamepadElements::Y)] = static_cast<float32>(gamepad.buttonY.isPressed);

    buf[static_cast<size_t>(eGamepadElements::LEFT_SHOULDER)] = static_cast<float32>(gamepad.leftShoulder.isPressed);
    buf[static_cast<size_t>(eGamepadElements::RIGHT_SHOULDER)] = static_cast<float32>(gamepad.rightShoulder.isPressed);

    float32 dpadX = 0.f;
    float32 dpadY = 0.f;
    if (gamepad.dpad.left.isPressed)
        dpadX = -1.f;
    else if (gamepad.dpad.right.isPressed)
        dpadX = 1.f;
    if (gamepad.dpad.down.isPressed)
        dpadY = -1.f;
    else if (gamepad.dpad.up.isPressed)
        dpadY = 1.f;

    buf[static_cast<size_t>(eGamepadElements::DPAD_X)] = dpadX;
    buf[static_cast<size_t>(eGamepadElements::DPAD_Y)] = dpadY;
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    if (controller == nullptr)
    {
        NSArray<GCController*>* controllers = [GCController controllers];
        if ([controllers count] != 0)
        {
            controller = (__bridge GCController*)CFBridgingRetain([controllers objectAtIndex:0]);
            gamepadDevice->profile = [controller extendedGamepad] != nil ? eGamepadProfiles::EXTENDED : eGamepadProfiles::SIMPLE;
        }
    }
    return controller != nullptr;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 id)
{
    bool removed = true;
    for (GCController* c in [GCController controllers])
    {
        if (c == controller)
        {
            removed = false;
            break;
        }
    }
    if (removed && controller != nullptr)
    {
        CFBridgingRelease(controller);
        controller = nullptr;
    }
    return controller != nullptr;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
