#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    \ingroup engine
    Engine run modes.
*/
enum class eEngineRunMode : int32
{
    GUI_STANDALONE = 0, //!< Run engine as standalone GUI application
    GUI_EMBEDDED, //!< Run engine inside other framework, e.g. Qt
    CONSOLE_MODE //!< Run engine as standalone console application
};

/**
    \ingroup engine
    Constants that name supported input device types.
*/
enum class eInputDevice : uint32
{
    UNKNOWN = 0, //!< Special value used in some case to specify that input device is unrecognized
    TOUCH_SURFACE, //!< Touch surface like touch screen on mobile devices
    MOUSE,
    KEYBOARD,
    GAMEPAD,
    PEN,
    TOUCH_PAD, //!< Touch pad which can be found on notebooks
};

/**
    \ingroup engine
    Constants that name mouse buttons.
*/
enum class eMouseButtons : uint32
{
    NONE = 0, //!< Special value used in some cases to specify that no mouse button is involved
    FIRST = 1,
    LEFT = FIRST,
    RIGHT = 2,
    MIDDLE = 3,
    EXTENDED1 = 4,
    EXTENDED2 = 5,
    LAST = EXTENDED2,

    COUNT = LAST, //!< Number of supported mouse buttons
};

/**
    \ingroup engine
    Modifier keys (shift, alt, control, etc) that accompany some input events (mouse, touch, key).
    Bitwise operators can be applied to enum members (|, |=, &, &=, ^, ^=).
*/
enum class eModifierKeys : uint32
{
    NONE = 0, //!< No modifier keys are pressed
    FIRST = 0x01,
    SHIFT = FIRST, //!< Any shift is pressed
    CONTROL = 0x02, //!< Any control is pressed
    ALT = 0x04, //!< Any alt is pressed
    COMMAND = 0x08, //!< Any command key is pressed (macOS only)
    LAST = COMMAND,
};

// clang-format off
/**
    \ingroup engine
    Define bitwise operators for strongly typed enums which can be used as bit flags.

    \code
    enum class E : int
    {
        FLAG1 = 0x01,
        FLAG2 = 0x02
    };
    DAVA_DEFINE_ENUM_OPERATORS(E)
    // Now you can use enum E without casting to int
    E e1 = E::FLAG1 | E::FLAG2;
    E e2 = e1 & ~E::FLAG1;
    e1 ^= e2;
    \endcode
*/
#define DAVA_DEFINE_ENUM_OPERATORS(enumType) \
    inline /*constexpr*/ enumType operator|(enumType l, enumType r) { return static_cast<enumType>(static_cast<uint32>(l) | static_cast<uint32>(r)); } \
    inline /*constexpr*/ enumType operator&(enumType l, enumType r) { return static_cast<enumType>(static_cast<uint32>(l) & static_cast<uint32>(r)); } \
    inline /*constexpr*/ enumType operator^(enumType l, enumType r) { return static_cast<enumType>(static_cast<uint32>(l) ^ static_cast<uint32>(r)); } \
    inline /*constexpr*/ enumType& operator|=(enumType& l, enumType r) { l = l | r; return l; } \
    inline /*constexpr*/ enumType& operator&=(enumType& l, enumType r) { l = l & r; return l; } \
    inline /*constexpr*/ enumType& operator^=(enumType& l, enumType r) { l = l ^ r; return l; } \
    inline /*constexpr*/ enumType operator~(enumType e) { return static_cast<enumType>(~static_cast<uint32>(e)); }
// clang-format on

DAVA_DEFINE_ENUM_OPERATORS(eModifierKeys)

} // namespace DAVA
