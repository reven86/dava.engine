#if !defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#import <GameController/GameController.h>

#include "GamepadDevice.h"
#include "UI/UIEvent.h"
#include "Input/InputSystem.h"

@interface GamepadIOS : NSObject
- (id)initWithParent:(DAVA::GamepadDevice*)parent;
@end

namespace DAVA
{
void GamepadDevice::InitInternal()
{
    if (NSFoundationVersionNumber > NSFoundationVersionNumber_iOS_6_1)
    {
        [[[GamepadIOS alloc] initWithParent:this] autorelease];
    }
}

void ProcessElementHandler(GamepadDevice* device, GamepadDevice::eDavaGamepadElement element, float32 value)
{
    device->SystemProcessElement(element, value);

    UIEvent newEvent;
    newEvent.element = element;
    newEvent.physPoint.x = value;
    newEvent.point.x = value;
    newEvent.phase = UIEvent::Phase::JOYSTICK;
    newEvent.device = eInputDevices::GAMEPAD;

    InputSystem::Instance()->ProcessInputEvent(&newEvent);
}

void GamepadDevice::OnControllerConnected(void* gameControllerObject)
{
    if (!gameControllerObject)
    {
        Reset();

        return;
    }

    SetAvailable(true);

    GCController* controller = (GCController*)gameControllerObject;
    [controller retain];

    profile = [controller extendedGamepad] ? GAMEPAD_PROFILE_EXTENDED : GAMEPAD_PROFILE_SIMPLE;

    if (controller.extendedGamepad)
    {
        controller.extendedGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_A, value);
        };
        controller.extendedGamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_B, value);
        };
        controller.extendedGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_X, value);
        };
        controller.extendedGamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_Y, value);
        };

        controller.extendedGamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_LS, value);
        };
        controller.extendedGamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_RS, value);
        };
        controller.extendedGamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_LT, value);
        };
        controller.extendedGamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_RT, value);
        };

        controller.extendedGamepad.leftThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_LX, value);
        };
        controller.extendedGamepad.leftThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_LY, value);
        };
        controller.extendedGamepad.rightThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_RX, value);
        };
        controller.extendedGamepad.rightThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_AXIS_RY, value);
        };

        controller.extendedGamepad.dpad.xAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_X, value);
        };
        controller.extendedGamepad.dpad.yAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_Y, value);
        };
    }
    else
    {
        controller.gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_A, value);
        };
        controller.gamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_B, value);
        };
        controller.gamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_X, value);
        };
        controller.gamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_Y, value);
        };

        controller.gamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_LS, value);
        };
        controller.gamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput* button, float value, BOOL pressed) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_BUTTON_RS, value);
        };

        controller.gamepad.dpad.xAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_X, value);
        };
        controller.gamepad.dpad.yAxis.valueChangedHandler = ^(GCControllerAxisInput* axis, float value) {
          ProcessElementHandler(this, GAMEPAD_ELEMENT_DPAD_Y, value);
        };
    }
}
}

@implementation GamepadIOS
{
@private
    bool isListening;
    DAVA::GamepadDevice* parent;
}

- (id)initWithParent:(DAVA::GamepadDevice*)_parent
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
    if (!isListening)
    {
        [self findController];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidConnect) name:@"GCControllerDidConnectNotification" object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidDisconnect) name:@"GCControllerDidDisconnectNotification" object:nil];
        isListening = YES;
    }
}

- (void)stopListening
{
    if (isListening)
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
    ((DAVA::GamepadDevice*)parent)->OnControllerConnected(0);
}

- (void)findController
{
    NSArray* controllers = [GCController controllers];
    if (controllers.count)
    {
        ((DAVA::GamepadDevice*)parent)->OnControllerConnected((void*)[controllers objectAtIndex:0]);
    }
}

@end

#endif
#endif // #if !__DAVAENGINE_COREV2__
