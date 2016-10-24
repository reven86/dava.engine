#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
class GamepadDevice;
namespace Private
{
struct MainDispatcherEvent;
class GamepadDeviceImpl final
{
public:
    GamepadDeviceImpl(GamepadDevice* gamepad);

    void Update();
    void ReadElements(float32 buf[], size_t size);

    void HandleGamepadMotion(const MainDispatcherEvent& e)
    {
    }
    void HandleGamepadButton(const MainDispatcherEvent& e)
    {
    }

    bool HandleGamepadAdded(uint32 id);
    bool HandleGamepadRemoved(uint32 id);

    GamepadDevice* gamepadDevice = nullptr;
    ::Windows::Gaming::Input::Gamepad ^ gamepad = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
