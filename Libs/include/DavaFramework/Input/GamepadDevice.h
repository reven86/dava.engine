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


#ifndef __GamepadDevice_H_
#define __GamepadDevice_H_

#include "Base/BaseObject.h"
#include "UI/UIEvent.h"

namespace DAVA
{
    class GamepadDevice;

    enum eDavaGamepadProfile
    {
        eDavaGamepadProfile_None,              // No device
        eDavaGamepadProfile_SimpleProfile,     // Logitech for iPhone 5/5s/5c
        eDavaGamepadProfile_ExtendedProfile    // Moga for iPhone 5/5s/5c, Moga for Android
    };


    enum eDavaGamepadElement
    {
        eDavaGamepadElement_LShoulderButton,
        eDavaGamepadElement_RShoulderButton,
        eDavaGamepadElement_LTrigger,
        eDavaGamepadElement_RTrigger,
        eDavaGamepadElement_ButtonA,
        eDavaGamepadElement_ButtonB,
        eDavaGamepadElement_ButtonX,
        eDavaGamepadElement_ButtonY,
        eDavaGamepadElement_LThumbstickX,
        eDavaGamepadElement_LThumbstickY,
        eDavaGamepadElement_RThumbstickX,
        eDavaGamepadElement_RThumbstickY,
        eDavaGamepadElement_DPadX,
        eDavaGamepadElement_DPadY
    };


    class GamepadDevice : public BaseObject
    {
    public:
#if defined(__DAVAENGINE_IPHONE__)
        GamepadDevice(void *gameController);
#elif defined(__DAVAENGINE_ANDROID__)
        GamepadDevice();
#else
        GamepadDevice();
#endif
        ~GamepadDevice();
        eDavaGamepadProfile GetProfile();
        float32 GetElementState(eDavaGamepadElement element);

    private:
        void ProcessElementStateChange(eDavaGamepadElement element, float32 value);
#if defined(__DAVAENGINE_IPHONE__)
        void *m_gameController;
#elif defined(__DAVAENGINE_ANDROID__)
        // not implemented
#endif
    };

}

#endif //__GamepadDevice_H_
