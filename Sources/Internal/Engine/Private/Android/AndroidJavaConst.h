#pragma once

#if defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
namespace Private
{
// Corresponding necessary constants from android.view.MotionEvent
struct AMotionEvent
{
    // Actions
    enum
    {
        ACTION_DOWN = 0,
        ACTION_UP = 1,
        ACTION_MOVE = 2,
        ACTION_POINTER_DOWN = 5,
        ACTION_POINTER_UP = 6,
        ACTION_HOVER_MOVE = 7,
        ACTION_SCROLL = 8,
    };

    // Mouse buttons
    enum
    {
        BUTTON_PRIMARY = 0x01,
        BUTTON_SECONDARY = 0x02,
        BUTTON_TERTIARY = 0x04,
    };

    // Joystick axes
    enum
    {
        AXIS_X = 0x00,
        AXIS_Y = 0x01,
        AXIS_Z = 0x0B,
        AXIS_RX = 0x0C,
        AXIS_RY = 0x0D,
        AXIS_RZ = 0x0E,
        AXIS_HAT_X = 0x0F,
        AXIS_HAT_Y = 0x10,
        AXIS_LTRIGGER = 0x11,
        AXIS_RTRIGGER = 0x12,
        AXIS_GAS = 0x16,
        AXIS_BRAKE = 0x17,
    };
};

// Corresponding necessary constants from android.view.KeyEvent
struct AKeyEvent
{
    // Actions
    enum
    {
        ACTION_DOWN = 0,
        ACTION_UP = 1,
    };

    // Keycodes
    enum
    {
        KEYCODE_BACK = 4,

        KEYCODE_DPAD_UP = 0x13,
        KEYCODE_DPAD_DOWN = 0x14,
        KEYCODE_DPAD_LEFT = 0x15,
        KEYCODE_DPAD_RIGHT = 0x16,

        // KEYCODE_DPAD_UP_LEFT = 0x10C,
        // KEYCODE_DPAD_DOWN_LEFT = 0x10D,
        // KEYCODE_DPAD_UP_RIGHT = 0x10E,
        // KEYCODE_DPAD_DOWN_RIGHT = 0x10F,

        KEYCODE_BUTTON_A = 0x60,
        KEYCODE_BUTTON_B = 0x61,
        KEYCODE_BUTTON_X = 0x63,
        KEYCODE_BUTTON_Y = 0x64,

        KEYCODE_BUTTON_L1 = 0x66,
        KEYCODE_BUTTON_R1 = 0x67,
        KEYCODE_BUTTON_L2 = 0x68,
        KEYCODE_BUTTON_R2 = 0x69,

        META_SHIFT_ON = 0x01,
        META_ALT_ON = 0x02,
        META_CTRL_ON = 0x00001000,
    };
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
