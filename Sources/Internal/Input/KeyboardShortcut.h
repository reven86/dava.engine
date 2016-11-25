#ifndef __DAVAENGINE_KEYBOARD_SHORTCUT_H__
#define __DAVAENGINE_KEYBOARD_SHORTCUT_H__

#include "Base/BaseTypes.h"
#include "Engine/EngineTypes.h"

#include "KeyboardDevice.h"

namespace DAVA
{
class KeyboardShortcut final
{
public:
    KeyboardShortcut();
    KeyboardShortcut(const KeyboardShortcut& shortcut);
#if defined(__DAVAENGINE_COREV2__)
    KeyboardShortcut(Key key_, eModifierKeys modifiers_ = eModifierKeys::NONE);
#else
    KeyboardShortcut(Key key, uint32 modifiers = 0);
#endif
    KeyboardShortcut(const String& str);

    ~KeyboardShortcut();

    KeyboardShortcut& operator=(const KeyboardShortcut& shortcut);
    bool operator==(const KeyboardShortcut& other) const;
    bool operator!=(const KeyboardShortcut& other) const;

    Key GetKey() const;
#if defined(__DAVAENGINE_COREV2__)
    eModifierKeys GetModifiers() const;
#else
    uint32 GetModifiers() const;
#endif

    String ToString() const;

private:
    Key key = Key::UNKNOWN;
#if defined(__DAVAENGINE_COREV2__)
    eModifierKeys modifiers = eModifierKeys::NONE;
#else
    uint32 modifiers = 0;
#endif
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