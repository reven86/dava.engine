#include "KeyboardShortcut.h"

#include "KeyboardDevice.h"
#include "InputSystem.h"
#include "Utils/StringUtils.h"
#include "Utils/Utils.h"

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

KeyboardShortcut::KeyboardShortcut(Key key_, uint32 modifiers_)
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
        String t = StringUtils::Trim(token);
        int modifier = 0;
        if (GlobalEnumMap<UIEvent::Modifier>::Instance()->ToValue(token.c_str(), modifier))
        {
            modifiers |= modifier;
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

uint32 KeyboardShortcut::GetModifiers() const
{
    return modifiers;
}

String KeyboardShortcut::ToString() const
{
    StringStream stream;

    int test = 0x01;
    while (test <= UIEvent::Modifier::LAST)
    {
        if (test & modifiers)
        {
            stream << GlobalEnumMap<UIEvent::Modifier>::Instance()->ToString(test) << "+";
        }
        test <<= 1;
    }

    stream << InputSystem::Instance()->GetKeyboard().GetKeyName(key);

    return stream.str();
}
}
