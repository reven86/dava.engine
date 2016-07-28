#ifndef __DAVAENGINE_KEYBOARD_SHORTCUT_H__
#define __DAVAENGINE_KEYBOARD_SHORTCUT_H__

#include "Base/BaseTypes.h"

#include "KeyboardDevice.h"

namespace DAVA
{
class KeyboardShortcut final
{
public:

    KeyboardShortcut();
    KeyboardShortcut(const KeyboardShortcut& shortcut);
    KeyboardShortcut(Key key, uint32 modifiers = 0);
    KeyboardShortcut(const String& str);

    ~KeyboardShortcut();

    KeyboardShortcut& operator=(const KeyboardShortcut& shortcut);
    bool operator==(const KeyboardShortcut& other) const;
    bool operator!=(const KeyboardShortcut& other) const;

    Key GetKey() const;
    uint32 GetModifiers() const;

    String ToString() const;

private:
    Key key = Key::UNKNOWN;
    uint32 modifiers = 0;
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