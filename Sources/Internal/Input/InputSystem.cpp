#include "InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Input/GamepadDevice.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
InputSystem::InputSystem()
    : keyboard(0)
    , gamepad(0)
    , isMultitouchEnabled(true)
{
    keyboard = new KeyboardDevice();
    gamepad = new GamepadDevice();
    mouse = new MouseDevice();
    AddInputCallback(InputCallback(UIControlSystem::Instance(), &UIControlSystem::OnInput, INPUT_DEVICE_KEYBOARD));
    pinCursor = false;
}

InputSystem::~InputSystem()
{
    SafeRelease(gamepad);
    SafeRelease(keyboard);
    SafeRelease(mouse);
}

void InputSystem::ProcessInputEvent(UIEvent* event)
{
    for (InputCallback& iCallBack : callbacks)
    {
        if (event->device == UIEvent::Device::KEYBOARD && (iCallBack.devices & INPUT_DEVICE_KEYBOARD))
        {
            iCallBack(event);
        }
        else if (event->device == UIEvent::Device::GAMEPAD && (iCallBack.devices & INPUT_DEVICE_JOYSTICK))
        {
            iCallBack(event);
        }
        else if ((event->device == UIEvent::Device::TOUCH_SURFACE || event->device == UIEvent::Device::MOUSE) && (iCallBack.devices & INPUT_DEVICE_TOUCH))
        {
            iCallBack(event);
        }
    }
}

void InputSystem::AddInputCallback(const InputCallback& inputCallback)
{
    callbacks.push_back(inputCallback);
}

bool InputSystem::RemoveInputCallback(const InputCallback& inputCallback)
{
    Vector<InputCallback>::iterator it = find(callbacks.begin(), callbacks.end(), inputCallback);
    if (it != callbacks.end())
        callbacks.erase(it);
    else
        return false;

    return true;
}

void InputSystem::RemoveAllInputCallbacks()
{
    callbacks.clear();
}

void InputSystem::OnBeforeUpdate()
{
}

void InputSystem::OnAfterUpdate()
{
    keyboard->OnFinishFrame();
}
};
