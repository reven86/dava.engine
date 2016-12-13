#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Base/RefPtr.h"
#include "Engine/EngineTypes.h"
#include "Functional/Function.h"

namespace DAVA
{
/**
    \defgroup input Input System
*/

class Engine;
class UIEvent;
class KeyboardDevice;
class GamepadDevice;
namespace Private
{
class EngineBackend;
struct MainDispatcherEvent;
}

class InputSystem final
{
    friend class Window;
    friend class Private::EngineBackend;
    friend class GamepadDevice;

public:
    // Temporal method for backward compatibility
    // TODO: remove InputSystem::Instance() method
    static InputSystem* Instance();

    uint32 AddHandler(eInputDevices inputDeviceMask, const Function<bool(UIEvent*)>& callback);
    void ChangeHandlerDeviceMask(uint32 token, eInputDevices newInputDeviceMask);
    void RemoveHandler(uint32 token);

    KeyboardDevice& GetKeyboard();
    GamepadDevice& GetGamepadDevice();

private:
    InputSystem(Engine* engine);
    ~InputSystem();

    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;

    void Update(float32 frameDelta);
    void OnAfterUpdate();
    void HandleInputEvent(UIEvent* uie);
    void HandleGamepadMotion(const Private::MainDispatcherEvent& e);
    void HandleGamepadButton(const Private::MainDispatcherEvent& e);

    void HandleGamepadAdded(const Private::MainDispatcherEvent& e);
    void HandleGamepadRemoved(const Private::MainDispatcherEvent& e);

private:
    RefPtr<KeyboardDevice> keyboard;
    RefPtr<GamepadDevice> gamepad;

    struct InputHandler
    {
        InputHandler(uint32 token_, eInputDevices inputDeviceMask_, const Function<bool(UIEvent*)>& callback_);

        uint32 token;
        eInputDevices inputDeviceMask;
        Function<bool(UIEvent*)> callback;
    };

    Vector<InputHandler> handlers;
    uint32 nextHandlerToken = 1;
    bool pendingHandlerRemoval = false;
};

inline InputSystem::InputHandler::InputHandler(uint32 token_, eInputDevices inputDeviceMask_, const Function<bool(UIEvent*)>& callback_)
    : token(token_)
    , inputDeviceMask(inputDeviceMask_)
    , callback(callback_)
{
}

inline KeyboardDevice& InputSystem::GetKeyboard()
{
    return *keyboard;
}

inline GamepadDevice& InputSystem::GetGamepadDevice()
{
    return *gamepad;
}

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Core/Core.h"
#include "UI/UIEvent.h"
#include "InputCallback.h"
#include "Input/MouseDevice.h"

/**
	\defgroup inputsystem	Input System
*/
namespace DAVA
{
class KeyboardDevice;
class GamepadDevice;

class InputSystem : public Singleton<InputSystem>
{
public:
    enum eInputDevice
    {
        INPUT_DEVICE_TOUCH = 1,
        INPUT_DEVICE_KEYBOARD = 1 << 1,
        INPUT_DEVICE_JOYSTICK = 1 << 2
    };

#if defined(__DAVAENGINE_COREV2__)
    friend class Private::EngineBackend;
#else
    friend void Core::CreateSingletons();
#endif

protected:
    ~InputSystem();
    /**
	 \brief Don't call this constructor!
	 */
    InputSystem();

public:
    void ProcessInputEvent(UIEvent* event);

    void AddInputCallback(const InputCallback& inputCallback);
    bool RemoveInputCallback(const InputCallback& inputCallback);
    void RemoveAllInputCallbacks();

    void OnBeforeUpdate();
    void OnAfterUpdate();

    inline KeyboardDevice& GetKeyboard();
    inline GamepadDevice& GetGamepadDevice();
    inline MouseDevice& GetMouseDevice();

    inline void EnableMultitouch(bool enabled);
    inline bool GetMultitouchEnabled() const;

protected:
    KeyboardDevice* keyboard;
    GamepadDevice* gamepad;
    MouseDevice* mouse;

    Vector<InputCallback> callbacks;
    bool pinCursor;

    bool isMultitouchEnabled;
};

inline KeyboardDevice& InputSystem::GetKeyboard()
{
    return *keyboard;
}

inline GamepadDevice& InputSystem::GetGamepadDevice()
{
    return *gamepad;
}

inline MouseDevice& InputSystem::GetMouseDevice()
{
    return *mouse;
}

inline void InputSystem::EnableMultitouch(bool enabled)
{
    isMultitouchEnabled = enabled;
}

inline bool InputSystem::GetMultitouchEnabled() const
{
    return isMultitouchEnabled;
}
};

#endif // !__DAVAENGINE_COREV2__
