#ifndef __DAVAENGINE_KEYBOARD_DEVICE_H__
#define __DAVAENGINE_KEYBOARD_DEVICE_H__

#include "Base/BaseObject.h"

/**
	\defgroup inputsystem	Input System
*/
namespace DAVA
{
enum class Key : uint32
{
    UNKNOWN = 0,
    ESCAPE,
    BACKSPACE,
    TAB,
    ENTER,
    SPACE,

    LSHIFT,
    LCTRL,
    LALT,

    LWIN,
    RWIN,
    APPS, // https://en.wikipedia.org/wiki/Menu_key

    PAUSE,
    CAPSLOCK,
    NUMLOCK,
    SCROLLLOCK,

    PGUP,
    PGDN,
    HOME,
    END,
    INSERT,
    DELETE,

    LEFT,
    UP,
    RIGHT,
    DOWN,

    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    GRAVE,
    MINUS,
    EQUALS,
    BACKSLASH,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    APOSTROPHE,
    COMMA,
    PERIOD,
    SLASH,

    NUMPAD0,
    NUMPAD1,
    NUMPAD2,
    NUMPAD3,
    NUMPAD4,
    NUMPAD5,
    NUMPAD6,
    NUMPAD7,
    NUMPAD8,
    NUMPAD9,

    MULTIPLY,
    DIVIDE,
    ADD,
    SUBTRACT,
    DECIMAL,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    BACK, // Android key
    MENU, // Android key

    NONUSBACKSLASH, // exist on some keyboards
    NUMPADENTER,
    PRINTSCREEN,
    RSHIFT,
    RCTRL,
    RALT,

    F13, // on mac - printscreen
    F14, // on mac - scrlock
    F15, // on mac - pause/break
    F16,
    F17,
    F18,
    F19,

    TOTAL_KEYS_COUNT
};

namespace Private
{
class EngineBackend;
}

class KeyboardDevice : public BaseObject
{
public:
    bool IsKeyPressed(Key key) const; // during frame
    const String& GetKeyName(Key key);
    void ClearAllKeys(); // unpress keys during ALT+TAB or similar events
private:
    friend class InputSystem;
    friend class CoreWin32Platform;
    friend class CorePlatformAndroid;
    friend class DavaQtKeyboard;
    friend class DavaQtApplyModifier;
    friend class QtLayer;
    friend class Private::EngineBackend;
#ifdef __DAVAENGINE_WIN_UAP__
    friend ref class WinUAPXamlApp;
#endif
    ~KeyboardDevice();
    KeyboardDevice();

#ifdef __DAVAENGINE_MACOS__
public:
#endif
    Key GetDavaKeyForSystemKey(uint32 systemKeyCode) const;
    void OnKeyPressed(Key keyCode);
    void OnKeyUnpressed(Key keyCode);
#ifdef __DAVAENGINE_MACOS__
private:
#endif
    void OnFinishFrame();

    void PrepareKeyTranslator();

    Bitset<static_cast<size_t>(Key::TOTAL_KEYS_COUNT)> currentFrameKeyStatus;
    Bitset<static_cast<size_t>(Key::TOTAL_KEYS_COUNT)> realKeyStatus;
    static const int MAX_KEYS = 512;
    Array<Key, MAX_KEYS> keyTranslator;
    Array<String, static_cast<size_t>(Key::TOTAL_KEYS_COUNT)> keyNames;
};

}; // end DAVA namespace

#endif
