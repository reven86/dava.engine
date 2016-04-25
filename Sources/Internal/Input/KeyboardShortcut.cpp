#include "KeyboardShortcut.h"

#include "KeyboardDevice.h"
#include "InputSystem.h"
#include "Utils/Utils.h"

namespace DAVA
{
const String KeyboardShortcut::SHIFT_NAME("SHIFT");
const String KeyboardShortcut::CTRL_NAME("CTRL");
const String KeyboardShortcut::ALT_NAME("ALT");
const String KeyboardShortcut::WIN_NAME("WIN");

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
    Vector<String> tokens;
    Split(str, "+", tokens);

    modifiers = 0;
    for (const String& token : tokens)
    {
        String t = Trim(token);
        if (t == SHIFT_NAME)
        {
            modifiers |= MODIFIER_SHIFT;
        }
        else if (t == CTRL_NAME)
        {
            modifiers |= MODIFIER_CTRL;
        }
        else if (t == ALT_NAME)
        {
            modifiers |= MODIFIER_ALT;
        }
        else if (t == WIN_NAME)
        {
            modifiers |= MODIFIER_WIN;
        }
        else
        {
            DVASSERT(key == Key::UNKNOWN);
            key = InputSystem::Instance()->GetKeyboard().GetKeyByName(t);
        }
    }

    DVASSERT(key != Key::UNKNOWN);
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
    StringStream stream;
    if ((modifiers & MODIFIER_SHIFT) != 0)
    {
        stream << SHIFT_NAME << "+";
    }
    if ((modifiers & MODIFIER_CTRL) != 0)
    {
        stream << CTRL_NAME << "+";
    }
    if ((modifiers & MODIFIER_ALT) != 0)
    {
        stream << ALT_NAME << "+";
    }
    if ((modifiers & MODIFIER_WIN) != 0)
    {
        stream << WIN_NAME << "+";
    }

    stream << InputSystem::Instance()->GetKeyboard().GetKeyName(key);

    return stream.str();
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

    case Key::RWIN:
    case Key::LWIN:
        return MODIFIER_WIN;

    default:
        return 0;
    }
}
}
