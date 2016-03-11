#include "KeyboardShortcut.h"

#include "KeyboardDevice.h"
#include "InputSystem.h"

namespace DAVA
{
KeyboardShortcut::KeyboardShortcut()
{
}

KeyboardShortcut::KeyboardShortcut(const KeyboardShortcut& shortcut)
    : key(shortcut.key)
    , modifiers(shortcut.modifiers)
{
}

KeyboardShortcut::KeyboardShortcut(Key key_, int32 modifiers_)
    : key(key_)
    , modifiers(modifiers_)
{
}

KeyboardShortcut::KeyboardShortcut(const String& str)
{
    key = InputSystem::Instance()->GetKeyboard().GetKeyByName(str);
    modifiers = 0;
}

KeyboardShortcut::~KeyboardShortcut()
{
}

KeyboardShortcut& KeyboardShortcut::operator=(const KeyboardShortcut& shortcut)
{
    key = shortcut.key;
    modifiers = shortcut.modifiers;
    return *this;
}

bool KeyboardShortcut::operator==(const KeyboardShortcut& other) const
{
    return key == other.key && modifiers == other.modifiers;
}

bool KeyboardShortcut::operator!=(const KeyboardShortcut& other) const
{
    return !(operator==(other));
}

Key KeyboardShortcut::GetKey() const
{
    return key;
}

int32 KeyboardShortcut::GetModifiers() const
{
    return modifiers;
}

String KeyboardShortcut::ToString() const
{
    return InputSystem::Instance()->GetKeyboard().GetKeyName(key);
}

int32 KeyboardShortcut::ConvertKeyToModifier(Key key)
{
    switch (key)
    {
    case Key::LSHIFT:
    case Key::RSHIFT:
        return MODIFIER_SHIFT;

    case Key::RALT:
    case Key::LALT:
        return MODIFIER_ALT;

    case Key::RCTRL:
    case Key::LCTRL:
        return MODIFIER_CTRL;

    default:
        return 0;
    }
}
}
