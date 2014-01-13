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


#import "GamepadDevice.h"
#import <GameController/GameController.h>
#include "UI/UIEvent.h"
#include "Input/InputSystem.h"

namespace DAVA
{
    UIEvent createJoystickEvent(int32 eventId, float64 time, float32 x, float32 y = 0.0f) {
        UIEvent newEvent;
        newEvent.tid = eventId;
        newEvent.physPoint.x = x;
        newEvent.physPoint.y = y;
        newEvent.point.x = x;
        newEvent.point.y = y;
        newEvent.phase = UIEvent::PHASE_JOYSTICK;
        newEvent.tapCount = 1;
        newEvent.timestamp = time;
        return newEvent;
    }


    GamepadDevice::GamepadDevice(void *gameController)
    {
        GCController *controller = (GCController *)gameController;
        [controller retain];
        m_gameController = controller;
        if(controller.extendedGamepad) {
            controller.extendedGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonA , value);
            };
            controller.extendedGamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonB , value);
            };
            controller.extendedGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonX , value);
            };
            controller.extendedGamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonY , value);
            };
            controller.extendedGamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_LShoulderButton , value);
            };
            controller.extendedGamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_RShoulderButton , value);
            };
            controller.extendedGamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_LTrigger , value);
            };
            controller.extendedGamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_RTrigger , value);
            };
        } else {
            controller.gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonA , value);
            };
            controller.gamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonB , value);
            };
            controller.gamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonX , value);
            };
            controller.gamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_ButtonY , value);
            };
            controller.gamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_LShoulderButton , value);
            };
            controller.gamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
                ProcessElementStateChange(DAVA::eDavaGamepadElement_RShoulderButton , value);
            };
        }
    }

    GamepadDevice::~GamepadDevice()
    {
        [(GCController *)m_gameController release];
    }

    void GamepadDevice::ProcessElementStateChange(eDavaGamepadElement element, float32 value) {
        UIEvent event = createJoystickEvent(element, 0.0, value);
        InputSystem::Instance()->ProcessInputEvent(&event);
    }

    eDavaGamepadProfile GamepadDevice::GetProfile()
    {
        return [(GCController *)m_gameController extendedGamepad] ? eDavaGamepadProfile_ExtendedProfile : eDavaGamepadProfile_SimpleProfile;
    }

    float32 GamepadDevice::GetElementState(eDavaGamepadElement element)
    {
        GCController *controller = (GCController *)m_gameController;
        switch(element) {
            case eDavaGamepadElement_ButtonA:
                return controller.gamepad.buttonA.value;
            case eDavaGamepadElement_ButtonB:
                return controller.gamepad.buttonB.value;
            case eDavaGamepadElement_ButtonX:
                return controller.gamepad.buttonX.value;
            case eDavaGamepadElement_ButtonY:
                return controller.gamepad.buttonY.value;
            case eDavaGamepadElement_LShoulderButton:
                return controller.gamepad.leftShoulder.value;
            case eDavaGamepadElement_RShoulderButton:
                return controller.gamepad.rightShoulder.value;
            case eDavaGamepadElement_DPadX:
                return controller.gamepad.dpad.xAxis.value;
            case eDavaGamepadElement_DPadY:
                return controller.gamepad.dpad.yAxis.value;
            case eDavaGamepadElement_LTrigger:
                return controller.extendedGamepad.leftTrigger.value;
            case eDavaGamepadElement_RTrigger:
                return controller.extendedGamepad.rightTrigger.value;
            case eDavaGamepadElement_LThumbstickX:
                return controller.extendedGamepad.leftThumbstick.xAxis.value;
            case eDavaGamepadElement_LThumbstickY:
                return controller.extendedGamepad.leftThumbstick.yAxis.value;
            case eDavaGamepadElement_RThumbstickX:
                return controller.extendedGamepad.rightThumbstick.xAxis.value;
            case eDavaGamepadElement_RThumbstickY:
                return controller.extendedGamepad.rightThumbstick.yAxis.value;
            default:
                return 0.0f; // ofc better to throw some exception
        }
    }

}
