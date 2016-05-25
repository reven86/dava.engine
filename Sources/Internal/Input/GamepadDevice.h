#ifndef __GAMEPAD_DEVICE_H_
#define __GAMEPAD_DEVICE_H_

#include "Base/BaseObject.h"

namespace DAVA
{
#pragma pack(push, 1)
class GamepadDevice : public BaseObject
{
public:
    enum eDavaGamepadProfile
    {
        GAMEPAD_PROFILE_SIMPLE = 0,
        GAMEPAD_PROFILE_NO_TRIGGERS,
        GAMEPAD_PROFILE_EXTENDED,

        GAMEPAD_PROFILE_COUNT
    };

    enum eDavaGamepadElement : uint32
    {
        GAMEPAD_ELEMENT_BUTTON_A = 0,
        GAMEPAD_ELEMENT_BUTTON_B,
        GAMEPAD_ELEMENT_BUTTON_X,
        GAMEPAD_ELEMENT_BUTTON_Y,
        GAMEPAD_ELEMENT_BUTTON_LS, //Left shoulder
        GAMEPAD_ELEMENT_BUTTON_RS, //Right shoulder

        GAMEPAD_ELEMENT_LT, //Left trigger
        GAMEPAD_ELEMENT_RT, //Right trigger

        GAMEPAD_ELEMENT_AXIS_LX, //Left joystick, axis X
        GAMEPAD_ELEMENT_AXIS_LY, //Left joystick, axis Y
        GAMEPAD_ELEMENT_AXIS_RX, //Right joystick, axis X
        GAMEPAD_ELEMENT_AXIS_RY, //Right joystick, axis Y

        GAMEPAD_ELEMENT_DPAD_X,
        GAMEPAD_ELEMENT_DPAD_Y,

        GAMEPAD_ELEMENT_COUNT
    };

    GamepadDevice();

    void Reset();

    inline bool IsAvailable() const;
    inline eDavaGamepadProfile GetProfile() const;
    inline float32 GetElementState(eDavaGamepadElement element) const;

    inline void SystemProcessElement(eDavaGamepadElement element, float32 newValue);

    void SetAvailable(bool available)
    {
        isAvailable = available;
    }

private:
    void InitInternal();

    float32 elementValues[GAMEPAD_ELEMENT_COUNT];
    eDavaGamepadProfile profile;

    bool isAvailable;

#if defined(__DAVAENGINE_IPHONE__)
public:
    void OnControllerConnected(void* gameControllerObject);
#endif

#if defined(__DAVAENGINE_ANDROID__)
public:
    static const uint8 INVALID_DAVAKEY = 0xFF;
    static const uint32 MAX_TRANSLATOR_KEYS = 256;

    inline uint8 GetDavaEventIdForSystemKeycode(int32 systemKey) const;
    inline uint8 GetDavaEventIdForSystemAxis(int32 systemKey) const;
    inline void OnTriggersAvailable(bool isAvailable);

private:
    uint8 keyTranslator[MAX_TRANSLATOR_KEYS];
    uint8 axisTranslator[MAX_TRANSLATOR_KEYS];
#endif
};
#pragma pack(pop)

inline bool GamepadDevice::IsAvailable() const
{
    return isAvailable;
}

inline GamepadDevice::eDavaGamepadProfile GamepadDevice::GetProfile() const
{
    return profile;
}

inline void GamepadDevice::SystemProcessElement(eDavaGamepadElement element, float32 value)
{
    if (element < GAMEPAD_ELEMENT_COUNT)
    {
        elementValues[element] = value;
    }
    else
    {
        DVASSERT(false && "unknown element");
    }
}

inline float32 GamepadDevice::GetElementState(eDavaGamepadElement element) const
{
    if (element < GAMEPAD_ELEMENT_COUNT)
    {
        return elementValues[element];
    }
    else
    {
        DVASSERT(false && "unknown element");
        return 0.f;
    }
}
    
#if defined(__DAVAENGINE_ANDROID__)
inline uint8 GamepadDevice::GetDavaEventIdForSystemKeycode(int32 systemKey) const
{
    if (systemKey < MAX_TRANSLATOR_KEYS && systemKey >= 0)
    {
        return keyTranslator[systemKey];
    }
    else
    {
        DVASSERT(false && "unknown systemKey");
        return INVALID_DAVAKEY;
    }
}

inline uint8 GamepadDevice::GetDavaEventIdForSystemAxis(int32 systemKey) const
{
    if (systemKey < MAX_TRANSLATOR_KEYS && systemKey >= 0)
    {
        return axisTranslator[systemKey];
    }
    else
    {
        DVASSERT(false && "unknown systemKey");
        return INVALID_DAVAKEY;
    }
}

inline void GamepadDevice::GamepadDevice::OnTriggersAvailable(bool isAvailable)
{
    if (isAvailable)
        profile = GAMEPAD_PROFILE_EXTENDED;
    else
        profile = GAMEPAD_PROFILE_NO_TRIGGERS;
}
#endif
}

#endif //__GAMEPAD_DEVICE_H_
