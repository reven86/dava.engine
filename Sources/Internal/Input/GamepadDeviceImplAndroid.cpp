#include "Input/GamepadDeviceImplAndroid.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/Android/AndroidJavaConst.h"
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

void GamepadDeviceImpl::HandleGamepadMotion(const MainDispatcherEvent& e)
{
    uint32 axis = e.gamepadEvent.axis;
    float32 value = e.gamepadEvent.value;

    // On game pads with two analog joysticks, axis AXIS_RX is often reinterpreted as absolute X position and
    // axis AXIS_RZ is reinterpreted as absolute Y position of the second joystick instead.
    eGamepadElements element = eGamepadElements::LEFT_THUMBSTICK_X;
    switch (axis)
    {
    case AMotionEvent::AXIS_X:
        element = eGamepadElements::LEFT_THUMBSTICK_X;
        break;
    case AMotionEvent::AXIS_Y:
        element = eGamepadElements::LEFT_THUMBSTICK_Y;
        break;
    case AMotionEvent::AXIS_Z:
    case AMotionEvent::AXIS_RX:
        element = eGamepadElements::RIGHT_THUMBSTICK_X;
        break;
    case AMotionEvent::AXIS_RY:
    case AMotionEvent::AXIS_RZ:
        element = eGamepadElements::RIGHT_THUMBSTICK_Y;
        break;
    case AMotionEvent::AXIS_LTRIGGER:
    case AMotionEvent::AXIS_BRAKE:
        element = eGamepadElements::LEFT_TRIGGER;
        break;
    case AMotionEvent::AXIS_RTRIGGER:
    case AMotionEvent::AXIS_GAS:
        element = eGamepadElements::RIGHT_TRIGGER;
        break;
    default:
        return;
    }

    // Android joystick Y-axis position is normalized to a range [-1, 1] where -1 for up or far and 1 for down or near.
    // Historically dava.engine's clients expect Y-axis value -1 for down or near and 1 for up and far so negate Y-axes.
    if (axis == AMotionEvent::AXIS_Y || axis == AMotionEvent::AXIS_RY || axis == AMotionEvent::AXIS_RZ)
    {
        value = -value;
    }

    size_t index = static_cast<size_t>(element);
    if (gamepadDevice->elementValues[index] != value)
    {
        gamepadDevice->elementValues[index] = value;
        gamepadDevice->elementTimestamps[index] = e.timestamp;
        gamepadDevice->elementChangedMask.set(index);
    }
}

void GamepadDeviceImpl::HandleGamepadButton(const MainDispatcherEvent& e)
{
    float32 value = e.type == MainDispatcherEvent::GAMEPAD_BUTTON_DOWN ? 1.f : 0.f;
    uint32 button = e.gamepadEvent.button;

    eGamepadElements element = eGamepadElements::A;
    switch (button)
    {
    case AKeyEvent::KEYCODE_DPAD_UP:
        element = eGamepadElements::DPAD_Y;
        break;
    case AKeyEvent::KEYCODE_DPAD_DOWN:
        element = eGamepadElements::DPAD_Y;
        break;
    case AKeyEvent::KEYCODE_DPAD_LEFT:
        element = eGamepadElements::DPAD_X;
        break;
    case AKeyEvent::KEYCODE_DPAD_RIGHT:
        element = eGamepadElements::DPAD_X;
        break;
    case AKeyEvent::KEYCODE_BUTTON_A:
        element = eGamepadElements::A;
        break;
    case AKeyEvent::KEYCODE_BUTTON_B:
        element = eGamepadElements::B;
        break;
    case AKeyEvent::KEYCODE_BUTTON_X:
        element = eGamepadElements::X;
        break;
    case AKeyEvent::KEYCODE_BUTTON_Y:
        element = eGamepadElements::Y;
        break;
    case AKeyEvent::KEYCODE_BUTTON_L1:
    case AKeyEvent::KEYCODE_BUTTON_L2:
        element = eGamepadElements::LEFT_SHOULDER;
        break;
    case AKeyEvent::KEYCODE_BUTTON_R1:
    case AKeyEvent::KEYCODE_BUTTON_R2:
        element = eGamepadElements::RIGHT_SHOULDER;
        break;
    default:
        return;
    }

    // Historically dava.engine normalize dpad presses into a range [-1, 1] where
    // -1 for dpad left or dpad down and 1 for dpad right or dpad up.
    if (button == AKeyEvent::KEYCODE_DPAD_DOWN || button == AKeyEvent::KEYCODE_DPAD_LEFT)
    {
        value = -value;
    }

    size_t index = static_cast<size_t>(element);
    if (gamepadDevice->elementValues[index] != value)
    {
        gamepadDevice->elementValues[index] = value;
        gamepadDevice->elementTimestamps[index] = e.timestamp;
        gamepadDevice->elementChangedMask.set(index);
    }
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 id)
{
    if (gamepadId == 0)
    {
        gamepadId = id;
    }
    return gamepadId != 0;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 id)
{
    if (gamepadId == id)
    {
        gamepadId = 0;
    }
    return gamepadId != 0;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
