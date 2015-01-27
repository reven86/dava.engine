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


#include "GamepadDevice.h"

namespace DAVA
{
#if defined(__DAVAENGINE_ANDROID__)
    const uint8 GamepadDevice::INVALID_DAVAKEY = 0xFF;
#endif
    
    GamepadDevice::GamepadDevice()
    {
        Reset();
        
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        InitInternal();
#endif
    }

    GamepadDevice::~GamepadDevice() { }
    
    void GamepadDevice::Reset()
    {
        isAvailable = false;
        profile = GAMEPAD_PROFILE_EXTENDED;
        Memset(elementValues, 0, sizeof(float32) * GAMEPAD_ELEMENT_COUNT);
    }
    
    GamepadDevice::eDavaGamepadProfile GamepadDevice::GetProfile()
    {
        return profile;
    }

    void GamepadDevice::SystemProcessElement(GamepadDevice::eDavaGamepadElement element, float32 value)
    {
        DVASSERT(element >= 0 && element < GAMEPAD_ELEMENT_COUNT);
        elementValues[element] = value;
    }
    
    float32 GamepadDevice::GetElementState(GamepadDevice::eDavaGamepadElement element)
    {
        DVASSERT(element >= 0 && element < GAMEPAD_ELEMENT_COUNT);
        return elementValues[element];
    }
    
#if defined(__DAVAENGINE_ANDROID__)
    void GamepadDevice::InitInternal()
    {
        Memset(keyTranslator, INVALID_DAVAKEY, MAX_TRANSLATOR_KEYS);
        
        keyTranslator[0x60] = (uint8)GAMEPAD_ELEMENT_BUTTON_A;     //BUTTON_A
        keyTranslator[0x61] = (uint8)GAMEPAD_ELEMENT_BUTTON_B;     //BUTTON_B
        keyTranslator[0x63] = (uint8)GAMEPAD_ELEMENT_BUTTON_X;     //BUTTON_X
        keyTranslator[0x64] = (uint8)GAMEPAD_ELEMENT_BUTTON_Y;     //BUTTON_Y
        
        keyTranslator[0x66] = (uint8)GAMEPAD_ELEMENT_BUTTON_LS;    //BUTTON_L1
        keyTranslator[0x67] = (uint8)GAMEPAD_ELEMENT_BUTTON_RS;    //BUTTON_R1
        keyTranslator[0x68] = (uint8)GAMEPAD_ELEMENT_LT;           //BUTTON_L2
        keyTranslator[0x69] = (uint8)GAMEPAD_ELEMENT_RT;           //BUTTON_R2
        
        keyTranslator[0x11] = (uint8)GAMEPAD_ELEMENT_LT;           //AXIS_LTRIGGER
        keyTranslator[0x12] = (uint8)GAMEPAD_ELEMENT_RT;           //AXIS_RTRIGGER
        
        keyTranslator[0x00] = (uint8)GAMEPAD_ELEMENT_AXIS_LX;      //AXIS_X
        keyTranslator[0x01] = (uint8)GAMEPAD_ELEMENT_AXIS_LY;      //AXIS_Y
        keyTranslator[0x0B] = (uint8)GAMEPAD_ELEMENT_AXIS_RX;      //AXIS_Z
        keyTranslator[0x0E] = (uint8)GAMEPAD_ELEMENT_AXIS_RY;      //AXIS_RZ
        
        keyTranslator[0x13] = (uint8)GAMEPAD_ELEMENT_DPAD_Y;       //DPAD_UP
        keyTranslator[0x14] = (uint8)GAMEPAD_ELEMENT_DPAD_Y;       //DPAD_DOWN
        keyTranslator[0x15] = (uint8)GAMEPAD_ELEMENT_DPAD_X;       //DPAD_LEFT
        keyTranslator[0x16] = (uint8)GAMEPAD_ELEMENT_DPAD_X;       //DPAD_RIGHT
    }
    
    GamepadDevice::eDavaGamepadElement GamepadDevice::GetDavaEventIdForSystemKey(int32 systemKey)
    {
        DVASSERT(systemKey < MAX_TRANSLATOR_KEYS);
        return (eDavaGamepadElement)keyTranslator[systemKey];
    }
#endif
}