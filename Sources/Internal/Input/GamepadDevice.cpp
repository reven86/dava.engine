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
    
    keyTranslator[0x60] = (uint8)GAMEPAD_ELEMENT_BUTTON_A;     //BUTTON_A
    keyTranslator[0x61] = (uint8)GAMEPAD_ELEMENT_BUTTON_B;     //BUTTON_B
    keyTranslator[0x63] = (uint8)GAMEPAD_ELEMENT_BUTTON_X;     //BUTTON_X
    keyTranslator[0x64] = (uint8)GAMEPAD_ELEMENT_BUTTON_Y;     //BUTTON_Y
    
    keyTranslator[0x66] = (uint8)GAMEPAD_ELEMENT_BUTTON_LS;    //BUTTON_L1
    keyTranslator[0x67] = (uint8)GAMEPAD_ELEMENT_BUTTON_RS;    //BUTTON_R1
    keyTranslator[0x68] = (uint8)GAMEPAD_ELEMENT_LT;           //BUTTON_L2
    keyTranslator[0x69] = (uint8)GAMEPAD_ELEMENT_RT;           //BUTTON_R2
    
    keyTranslator[0x13] = (uint8)GAMEPAD_ELEMENT_DPAD_Y;       //DPAD_UP
    keyTranslator[0x14] = (uint8)GAMEPAD_ELEMENT_DPAD_Y;       //DPAD_DOWN
    keyTranslator[0x15] = (uint8)GAMEPAD_ELEMENT_DPAD_X;       //DPAD_LEFT
    keyTranslator[0x16] = (uint8)GAMEPAD_ELEMENT_DPAD_X;       //DPAD_RIGHT
    
    axisTranslator[0x11] = (uint8)GAMEPAD_ELEMENT_LT;           //AXIS_LTRIGGER
    axisTranslator[0x12] = (uint8)GAMEPAD_ELEMENT_RT;           //AXIS_RTRIGGER
    axisTranslator[0x17] = (uint8)GAMEPAD_ELEMENT_LT;           //AXIS_BRAKE
    axisTranslator[0x16] = (uint8)GAMEPAD_ELEMENT_RT;           //AXIS_GAS
    
    axisTranslator[0x00] = (uint8)GAMEPAD_ELEMENT_AXIS_LX;      //AXIS_X
    axisTranslator[0x01] = (uint8)GAMEPAD_ELEMENT_AXIS_LY;      //AXIS_Y
    axisTranslator[0x0B] = (uint8)GAMEPAD_ELEMENT_AXIS_RX;      //AXIS_Z
    axisTranslator[0x0E] = (uint8)GAMEPAD_ELEMENT_AXIS_RY;      //AXIS_RZ
    axisTranslator[0x0C] = (uint8)GAMEPAD_ELEMENT_AXIS_RX;      //AXIS_RX
    axisTranslator[0x0D] = (uint8)GAMEPAD_ELEMENT_AXIS_RY;      //AXIS_RY
}
#endif

}