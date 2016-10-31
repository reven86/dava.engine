#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

DAVA_FORWARD_DECLARE_OBJC_CLASS(GCController);
DAVA_FORWARD_DECLARE_OBJC_CLASS(GCExtendedGamepad);
DAVA_FORWARD_DECLARE_OBJC_CLASS(GCGamepad);

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
    void ReadExtendedGamepadElements(GCExtendedGamepad* gamepad, float32 buf[]);
    void ReadGamepadElements(GCGamepad* gamepad, float32 buf[]);

    void HandleGamepadMotion(const MainDispatcherEvent&)
    {
    }
    void HandleGamepadButton(const MainDispatcherEvent&)
    {
    }

    bool HandleGamepadAdded(uint32 id);
    bool HandleGamepadRemoved(uint32 id);

    GamepadDevice* gamepadDevice = nullptr;
    GCController* controller = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
