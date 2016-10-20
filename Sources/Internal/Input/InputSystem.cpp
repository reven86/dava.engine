#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Input/GamepadDevice.h"
#include "Input/MouseDevice.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"

namespace DAVA
{
InputSystem* InputSystem::Instance()
{
    return Engine::Instance()->GetContext()->inputSystem;
}

InputSystem::InputSystem(UIControlSystem* uiControlSystem_)
    : uiControlSystem(uiControlSystem_)
    , keyboard(new KeyboardDevice())
    , gamepad(new GamepadDevice())
    , mouse(new MouseDevice())
{
}

InputSystem::~InputSystem() = default;

void InputSystem::HandleInputEvent(UIEvent* uie)
{
    bool handled = false;
    for (const InputHandler& h : handlers)
    {
        if ((h.inputDeviceMask & uie->device) != eInputDevice::UNKNOWN)
        {
            handled |= h.callback(uie);
            if (handled)
            {
                break;
            }
        }
    }
    if (!handled && uiControlSystem != nullptr)
    {
        uiControlSystem->OnInput(uie);
    }
}

uint32 InputSystem::AddHandler(eInputDevice inputDeviceMask, const Function<bool(UIEvent*)>& callback)
{
    DVASSERT(callback != nullptr);

    uint32 token = nextHandlerToken;
    nextHandlerToken += 1;
    handlers.emplace_back(token, inputDeviceMask, callback);
    return token;
}

void InputSystem::ChangeHandlerDeviceMask(uint32 token, eInputDevice newInputDeviceMask)
{
    auto it = std::find_if(begin(handlers), end(handlers), [token](const InputHandler& o) { return o.token == token; });
    if (it != end(handlers))
    {
        it->inputDeviceMask = newInputDeviceMask;
    }
}

void InputSystem::RemoveHandler(uint32 token)
{
    DVASSERT(token != 0);

    auto it = std::find_if(begin(handlers), end(handlers), [token](const InputHandler& o) { return o.token == token; });
    if (it != end(handlers))
    {
        it->token = 0;
        it->inputDeviceMask = eInputDevice::UNKNOWN;
        pendingHandlerRemoval = true;
    }
}

void InputSystem::OnAfterUpdate()
{
    keyboard->OnFinishFrame();
    if (pendingHandlerRemoval)
    {
        handlers.erase(std::remove_if(begin(handlers), end(handlers), [](const InputHandler& h) { return h.token == 0; }), end(handlers));
        pendingHandlerRemoval = false;
    }
}

void InputSystem::HandleGamepadMotion(const Private::MainDispatcherEvent& e)
{
    uint32 axis = e.gamepadEvent.axis;
    float32 value = e.gamepadEvent.value;
    uint32 davaKey = gamepad->GetDavaEventIdForSystemAxis(axis);
    if (davaKey != GamepadDevice::INVALID_DAVAKEY)
    {
        UIEvent uie;
        uie.element = static_cast<GamepadDevice::eDavaGamepadElement>(davaKey);
        uie.physPoint.x = value;
        uie.point.x = value;
        uie.phase = UIEvent::Phase::JOYSTICK;
        uie.device = eInputDevice::GAMEPAD;
        uie.timestamp = e.timestamp / 1000.0;

        gamepad->SystemProcessElement(static_cast<GamepadDevice::eDavaGamepadElement>(davaKey), value);
        HandleInputEvent(&uie);
    }
}

void InputSystem::HandleGamepadButton(const Private::MainDispatcherEvent& e)
{
    uint32 button = e.gamepadEvent.button;
    float32 value = e.gamepadEvent.value;
    uint32 davaKey = gamepad->GetDavaEventIdForSystemKeycode(button);
    if (davaKey != GamepadDevice::INVALID_DAVAKEY)
    {
        UIEvent uie;
        uie.element = static_cast<GamepadDevice::eDavaGamepadElement>(davaKey);
        uie.physPoint.x = value;
        uie.point.x = value;
        uie.phase = UIEvent::Phase::JOYSTICK;
        uie.device = eInputDevice::GAMEPAD;
        uie.timestamp = e.timestamp / 1000.0;

        gamepad->SystemProcessElement(static_cast<GamepadDevice::eDavaGamepadElement>(davaKey), value);
        HandleInputEvent(&uie);
    }
}

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

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

#endif // !__DAVAENGINE_COREV2__
