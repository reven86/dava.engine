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

    keyTranslator[0x60] = (uint8)GAMEPAD_ELEMENT_BUTTON_A; //BUTTON_A
    keyTranslator[0x61] = (uint8)GAMEPAD_ELEMENT_BUTTON_B; //BUTTON_B
    keyTranslator[0x63] = (uint8)GAMEPAD_ELEMENT_BUTTON_X; //BUTTON_X
    keyTranslator[0x64] = (uint8)GAMEPAD_ELEMENT_BUTTON_Y; //BUTTON_Y

    keyTranslator[0x66] = (uint8)GAMEPAD_ELEMENT_BUTTON_LS; //BUTTON_L1
    keyTranslator[0x67] = (uint8)GAMEPAD_ELEMENT_BUTTON_RS; //BUTTON_R1
    keyTranslator[0x68] = (uint8)GAMEPAD_ELEMENT_LT; //BUTTON_L2
    keyTranslator[0x69] = (uint8)GAMEPAD_ELEMENT_RT; //BUTTON_R2

    keyTranslator[0x13] = (uint8)GAMEPAD_ELEMENT_DPAD_Y; //DPAD_UP
    keyTranslator[0x14] = (uint8)GAMEPAD_ELEMENT_DPAD_Y; //DPAD_DOWN
    keyTranslator[0x15] = (uint8)GAMEPAD_ELEMENT_DPAD_X; //DPAD_LEFT
    keyTranslator[0x16] = (uint8)GAMEPAD_ELEMENT_DPAD_X; //DPAD_RIGHT

    axisTranslator[17] = (uint8)GAMEPAD_ELEMENT_LT; //AXIS_LTRIGGER
    axisTranslator[18] = (uint8)GAMEPAD_ELEMENT_RT; //AXIS_RTRIGGER
    axisTranslator[23] = (uint8)GAMEPAD_ELEMENT_LT; //AXIS_BRAKE
    axisTranslator[22] = (uint8)GAMEPAD_ELEMENT_RT; //AXIS_GAS

    axisTranslator[0] = (uint8)GAMEPAD_ELEMENT_AXIS_LX; //AXIS_X
    axisTranslator[1] = (uint8)GAMEPAD_ELEMENT_AXIS_LY; //AXIS_Y
    axisTranslator[11] = (uint8)GAMEPAD_ELEMENT_AXIS_RX; //AXIS_Z
    axisTranslator[14] = (uint8)GAMEPAD_ELEMENT_AXIS_RY; //AXIS_RZ
    axisTranslator[12] = (uint8)GAMEPAD_ELEMENT_AXIS_RX; //AXIS_RX
    axisTranslator[13] = (uint8)GAMEPAD_ELEMENT_AXIS_RY; //AXIS_RY
}
#endif
}
