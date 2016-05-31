#ifndef __DAVAENGINE_KEYBOARD_SHORTCUT_H__
#define __DAVAENGINE_KEYBOARD_SHORTCUT_H__

#include "Base/BaseTypes.h"

#include "KeyboardDevice.h"

namespace DAVA
{
class KeyboardShortcut final
{
public:
    enum Modifier
    {
        MODIFIER_SHIFT = 0x01,
        MODIFIER_CTRL = 0x02,
        MODIFIER_ALT = 0x04,
        MODIFIER_WIN = 0x08,

        LAST_MODIFIER = 0x08
    };

    KeyboardShortcut();
    KeyboardShortcut(const KeyboardShortcut& shortcut);
    KeyboardShortcut(Key key, int32 modifiers = 0);
    KeyboardShortcut(const String& str);

    ~KeyboardShortcut();

    KeyboardShortcut& operator=(const KeyboardShortcut& shortcut);
    bool operator==(const KeyboardShortcut& other) const;
    bool operator!=(const KeyboardShortcut& other) const;

    Key GetKey() const;
    int32 GetModifiers() const;

    String ToString() const;

    static int32 ConvertKeyToModifier(Key key);

private:
    Key key = Key::UNKNOWN;
    int32 modifiers = 0;
};
}

namespace std
{
template <>
struct hash<DAVA::KeyboardShortcut>
{
    size_t operator()(const DAVA::KeyboardShortcut& shortcut) const
    {
        return static_cast<size_t>(shortcut.GetKey()) | (static_cast<size_t>(shortcut.GetModifiers()) << 16);
    }
};
}

#endif //__DAVAENGINE_KEYBOARD_SHORTCUT_H__