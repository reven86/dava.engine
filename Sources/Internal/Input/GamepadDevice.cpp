#include "GamepadDevice.h"

namespace DAVA
{
GamepadDevice::GamepadDevice()
{
    Reset();

    InitInternal();
}

void GamepadDevice::Reset()
{
    isAvailable = false;
    profile = GAMEPAD_PROFILE_EXTENDED;
    Memset(elementValues, 0, sizeof(float32) * GAMEPAD_ELEMENT_COUNT);
}

#if defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_MACOS__)
void GamepadDevice::InitInternal()
{
}
#endif

#if defined(__DAVAENGINE_ANDROID__)
void GamepadDevice::InitInternal()
{
    Memset(keyTranslator, INVALID_DAVAKEY, MAX_TRANSLATOR_KEYS);
    Memset(axisTranslator, INVALID_DAVAKEY, MAX_TRANSLATOR_KEYS);

    keyTranslator[0x60] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_A); //BUTTON_A
    keyTranslator[0x61] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_B); //BUTTON_B
    keyTranslator[0x63] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_X); //BUTTON_X
    keyTranslator[0x64] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_Y); //BUTTON_Y

    keyTranslator[0x66] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_LS); //BUTTON_L1
    keyTranslator[0x67] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_RS); //BUTTON_R1
    keyTranslator[0x68] = static_cast<uint8>(GAMEPAD_ELEMENT_LT); //BUTTON_L2
    keyTranslator[0x69] = static_cast<uint8>(GAMEPAD_ELEMENT_RT); //BUTTON_R2

    keyTranslator[0x13] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_Y); //DPAD_UP
    keyTranslator[0x14] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_Y); //DPAD_DOWN
    keyTranslator[0x15] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_X); //DPAD_LEFT
    keyTranslator[0x16] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_X); //DPAD_RIGHT

    axisTranslator[17] = static_cast<uint8>(GAMEPAD_ELEMENT_LT); //AXIS_LTRIGGER
    axisTranslator[18] = static_cast<uint8>(GAMEPAD_ELEMENT_RT); //AXIS_RTRIGGER
    axisTranslator[23] = static_cast<uint8>(GAMEPAD_ELEMENT_LT); //AXIS_BRAKE
    axisTranslator[22] = static_cast<uint8>(GAMEPAD_ELEMENT_RT); //AXIS_GAS

    axisTranslator[0] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_LX); //AXIS_X
    axisTranslator[1] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_LY); //AXIS_Y
    axisTranslator[11] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RX); //AXIS_Z
    axisTranslator[14] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RY); //AXIS_RZ
    axisTranslator[12] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RX); //AXIS_RX
    axisTranslator[13] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RY); //AXIS_RY
}
#endif
}
