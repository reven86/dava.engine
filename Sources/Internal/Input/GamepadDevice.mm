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


#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <GameController/GameController.h>

#include "GamepadDevice.h"
#include "UI/UIEvent.h"
#include "Input/InputSystem.h"

@interface GamepadIOS : NSObject
- (id)initWithParent:(DAVA::GamepadDevice *)parent;
@end

namespace DAVA
{

void GamepadDevice::InitInternal()
{
    if(NSFoundationVersionNumber > NSFoundationVersionNumber_iOS_6_1)
    {
        [[[GamepadIOS alloc] initWithParent:this] autorelease];
    }
}

void ProcessElementHandler(GamepadDevice * device, GamepadDevice::eDavaGamepadElement element, float32 value)
{
    device->SystemProcessElement(element, value);
    
    UIEvent newEvent;
    newEvent.tid = element;
    newEvent.physPoint.x = value;
    newEvent.point.x = value;
    newEvent.phase = UIEvent::Phase::JOYSTICK;
    newEvent.device = UIEvent::Device::GAMEPAD;
    newEvent.tapCount = 1;
    
    InputSystem::Instance()->ProcessInputEvent(&newEvent);
}

void GamepadDevice::OnControllerConnected(void * gameControllerObject)
{
    if(!gameControllerObject)
    {
        Reset();
        
        return;
    }
    
    SetAvailable(true);
    
    GCController *controller = (GCController *)gameControllerObject;
    [controller retain];
    
    profile = [controller extendedGamepad] ? GAMEPAD_PROFILE_EXTENDED : GAMEPAD_PROFILE_SIMPLE;
    
    if(controller.extendedGamepad)
    {
        controller.extendedGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_A, value);
        };
        controller.extendedGamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_B, value);
        };
        controller.extendedGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_X , value);
        };
        controller.extendedGamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_Y, value);
        };
        
        controller.extendedGamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_LS, value);
        };
        controller.extendedGamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_RS, value);
        };
        controller.extendedGamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_LT, value);
        };
        controller.extendedGamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_RT, value);
        };
        
        controller.extendedGamepad.leftThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_LX, value);
        };
        controller.extendedGamepad.leftThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_LY, value);
        };
        controller.extendedGamepad.rightThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_RX, value);
        };
        controller.extendedGamepad.rightThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_RY, value);
        };
        
        controller.extendedGamepad.dpad.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_X, value);
        };
        controller.extendedGamepad.dpad.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_Y, value);
        };
    }
    else
    {
        controller.gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_A, value);
        };
        controller.gamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_B, value);
        };
        controller.gamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_X , value);
        };
        controller.gamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_Y, value);
        };
        
        controller.gamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_LS, value);
        };
        controller.gamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_RS, value);
        };
        
        controller.gamepad.dpad.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_X, value);
        };
        controller.gamepad.dpad.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
            ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_Y, value);
        };
    }
}
    
}

@implementation GamepadIOS
{
@private
    bool isListening;
    DAVA::GamepadDevice *parent;
}

- (id)initWithParent:(DAVA::GamepadDevice *)_parent
{
    self = [super init];
    if (self)
    {
        isListening = NO;
        parent = _parent;
        [self startListening];
    }
    
    return self;
}

- (void)dealloc
{
    [self stopListening];
    [super dealloc];
}

- (void)startListening
{
    if(!isListening)
    {
        [self findController];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidConnect) name:@"GCControllerDidConnectNotification" object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidDisconnect) name:@"GCControllerDidDisconnectNotification" object:nil];
        isListening = YES;
    }
}

- (void)stopListening
{
    if(isListening)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        isListening = NO;
    }
}

- (void)gameControllerDidConnect
{
    [self findController];
}

- (void)gameControllerDidDisconnect
{
    ((DAVA::GamepadDevice *)parent)->OnControllerConnected(0);
}

- (void)findController
{
    NSArray *controllers = [GCController controllers];
    if(controllers.count)
    {
        ((DAVA::GamepadDevice *)parent)->OnControllerConnected((void *)[controllers objectAtIndex:0]);
    }
}

@end

#endif