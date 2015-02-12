/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __GAMEPAD_DEVICE_H_
#define __GAMEPAD_DEVICE_H_

#include "Base/BaseObject.h"
#include "UI/UIEvent.h"

namespace DAVA
{
    
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
        
        
    enum eDavaGamepadElement
    {
        GAMEPAD_ELEMENT_BUTTON_A = 0,
        GAMEPAD_ELEMENT_BUTTON_B,
        GAMEPAD_ELEMENT_BUTTON_X,
        GAMEPAD_ELEMENT_BUTTON_Y,
        GAMEPAD_ELEMENT_BUTTON_LS,  //Left shoulder
        GAMEPAD_ELEMENT_BUTTON_RS,  //Right shoulder
            
        GAMEPAD_ELEMENT_LT,         //Left trigger
        GAMEPAD_ELEMENT_RT,         //Right trigger
            
        GAMEPAD_ELEMENT_AXIS_LX,    //Left joystick, axis X
        GAMEPAD_ELEMENT_AXIS_LY,    //Left joystick, axis Y
        GAMEPAD_ELEMENT_AXIS_RX,    //Right joystick, axis X
        GAMEPAD_ELEMENT_AXIS_RY,    //Right joystick, axis Y
            
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

    void SetAvailable(bool available) { isAvailable = available; }

private:
    void InitInternal();

    float32 elementValues[GAMEPAD_ELEMENT_COUNT];
    eDavaGamepadProfile profile;
        
    bool isAvailable;

#if defined(__DAVAENGINE_IPHONE__)
public:
    void OnControllerConnected(void * gameControllerObject);
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
    
inline bool GamepadDevice::IsAvailable() const
{
    return isAvailable;
}

inline GamepadDevice::eDavaGamepadProfile GamepadDevice::GetProfile() const
{
    return profile;
}

inline void GamepadDevice::SystemProcessElement(GamepadDevice::eDavaGamepadElement element, float32 value)
{
    DVASSERT(element >= 0 && element < GAMEPAD_ELEMENT_COUNT);
    elementValues[element] = value;
}

inline float32 GamepadDevice::GetElementState(GamepadDevice::eDavaGamepadElement element) const
{
    DVASSERT(element >= 0 && element < GAMEPAD_ELEMENT_COUNT);
    return elementValues[element];
}
    
#if defined(__DAVAENGINE_ANDROID__)
inline uint8 GamepadDevice::GetDavaEventIdForSystemKeycode(int32 systemKey) const
{
    DVASSERT(systemKey < MAX_TRANSLATOR_KEYS);
    return keyTranslator[systemKey];
}

inline uint8 GamepadDevice::GetDavaEventIdForSystemAxis(int32 systemKey) const
{
    DVASSERT(systemKey < MAX_TRANSLATOR_KEYS);
    return axisTranslator[systemKey];
}

inline void GamepadDevice::GamepadDevice::OnTriggersAvailable(bool isAvailable)
{
    if(isAvailable)
        profile = GAMEPAD_PROFILE_EXTENDED;
    else
        profile = GAMEPAD_PROFILE_NO_TRIGGERS;
}
#endif
    
}

#endif //__GAMEPAD_DEVICE_H_
