#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
class GamepadDevice;
namespace Private
{
struct MainDispatcherEvent;
class GamepadDeviceImpl final
{
public:
    GamepadDeviceImpl(GamepadDevice* gamepadDevice);

    void Update();

    void HandleGamepadMotion(const MainDispatcherEvent& e);
    void HandleGamepadButton(const MainDispatcherEvent& e);

    bool HandleGamepadAdded(uint32 id);
    bool HandleGamepadRemoved(uint32 id);

    GamepadDevice* gamepadDevice = nullptr;
    uint32 gamepadId = 0;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
