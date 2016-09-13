#ifndef __DAVAENGINE_INPUT_SYSTEM_H__
#define __DAVAENGINE_INPUT_SYSTEM_H__

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
#if !defined(__DAVAENGINE_COREV2__)
    inline MouseDevice& GetMouseDevice();
#endif // !defined(__DAVAENGINE_COREV2__)

    inline void EnableMultitouch(bool enabled);
    inline bool GetMultitouchEnabled() const;

protected:
    KeyboardDevice* keyboard;
    GamepadDevice* gamepad;
#if !defined(__DAVAENGINE_COREV2__)
    MouseDevice* mouse;
#endif // !defined(__DAVAENGINE_COREV2__)

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

#if !defined(__DAVAENGINE_COREV2__)
inline MouseDevice& InputSystem::GetMouseDevice()
{
    return *mouse;
}
#endif // !defined(__DAVAENGINE_COREV2__)

inline void InputSystem::EnableMultitouch(bool enabled)
{
    isMultitouchEnabled = enabled;
}

inline bool InputSystem::GetMultitouchEnabled() const
{
    return isMultitouchEnabled;
}
};

#endif
