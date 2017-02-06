#include "Input/Private/Win10/GamepadDeviceImplWin10.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

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
    ReadElements(readBuf, GamepadDevice::ELEMENT_COUNT);

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

void GamepadDeviceImpl::ReadElements(float32 buf[], size_t size)
{
    DVASSERT(size == GamepadDevice::ELEMENT_COUNT);

    using ::Windows::Gaming::Input::GamepadReading;
    using ::Windows::Gaming::Input::GamepadButtons;

    GamepadReading reading = gamepad->GetCurrentReading();

    buf[static_cast<size_t>(eGamepadElements::A)] = static_cast<float32>((reading.Buttons & GamepadButtons::A) != GamepadButtons::None);
    buf[static_cast<size_t>(eGamepadElements::B)] = static_cast<float32>((reading.Buttons & GamepadButtons::B) != GamepadButtons::None);
    buf[static_cast<size_t>(eGamepadElements::X)] = static_cast<float32>((reading.Buttons & GamepadButtons::X) != GamepadButtons::None);
    buf[static_cast<size_t>(eGamepadElements::Y)] = static_cast<float32>((reading.Buttons & GamepadButtons::Y) != GamepadButtons::None);

    buf[static_cast<size_t>(eGamepadElements::LEFT_SHOULDER)] = static_cast<float32>((reading.Buttons & GamepadButtons::LeftShoulder) != GamepadButtons::None);
    buf[static_cast<size_t>(eGamepadElements::RIGHT_SHOULDER)] = static_cast<float32>((reading.Buttons & GamepadButtons::RightShoulder) != GamepadButtons::None);

    buf[static_cast<size_t>(eGamepadElements::LEFT_THUMBSTICK_X)] = static_cast<float32>(reading.LeftThumbstickX);
    buf[static_cast<size_t>(eGamepadElements::LEFT_THUMBSTICK_Y)] = static_cast<float32>(reading.LeftThumbstickY);
    buf[static_cast<size_t>(eGamepadElements::RIGHT_THUMBSTICK_X)] = static_cast<float32>(reading.RightThumbstickX);
    buf[static_cast<size_t>(eGamepadElements::RIGHT_THUMBSTICK_Y)] = static_cast<float32>(reading.RightThumbstickY);

    buf[static_cast<size_t>(eGamepadElements::LEFT_TRIGGER)] = static_cast<float32>(reading.LeftTrigger);
    buf[static_cast<size_t>(eGamepadElements::RIGHT_TRIGGER)] = static_cast<float32>(reading.RightTrigger);

    float32 dpadX = 0.f;
    float32 dpadY = 0.f;
    if ((reading.Buttons & GamepadButtons::DPadLeft) != GamepadButtons::None)
        dpadX = -1.f;
    else if ((reading.Buttons & GamepadButtons::DPadRight) != GamepadButtons::None)
        dpadX = 1.f;
    if ((reading.Buttons & GamepadButtons::DPadDown) != GamepadButtons::None)
        dpadY = -1.f;
    else if ((reading.Buttons & GamepadButtons::DPadUp) != GamepadButtons::None)
        dpadY = 1.f;

    buf[static_cast<size_t>(eGamepadElements::DPAD_X)] = dpadX;
    buf[static_cast<size_t>(eGamepadElements::DPAD_Y)] = dpadY;
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    // Only one connected gamepad is supported by dava.engine for now.
    // So choose first gamepad from gamepad list.

    using ::Windows::Foundation::Collections::IVectorView;
    using ::Windows::Gaming::Input::Gamepad;

    if (gamepad == nullptr)
    {
        IVectorView<Gamepad ^> ^ gamepads = Gamepad::Gamepads;
        if (gamepads->Size != 0)
        {
            gamepad = gamepads->GetAt(0);
            gamepadDevice->profile = eGamepadProfiles::EXTENDED;
        }
    }
    return gamepad != nullptr;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 /*id*/)
{
    // Only one connected gamepad is supported by dava.engine for now.
    // So check whether currently using gamepad is removed

    using ::Windows::Foundation::Collections::IVectorView;
    using ::Windows::Gaming::Input::Gamepad;

    bool removed = true;
    IVectorView<Gamepad ^> ^ gamepads = Gamepad::Gamepads;
    for (unsigned int i = 0, n = gamepads->Size; i < n; ++i)
    {
        if (gamepads->GetAt(i) == gamepad)
        {
            removed = false;
            break;
        }
    }
    if (removed)
    {
        gamepad = nullptr;
    }
    return gamepad != nullptr;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
