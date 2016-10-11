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

#if defined(__DAVAENGINE_COREV2__)
KeyboardShortcut::KeyboardShortcut(Key key_, eModifierKeys modifiers_)
#else
KeyboardShortcut::KeyboardShortcut(Key key_, uint32 modifiers_)
#endif
    : key(key_)
    , modifiers(modifiers_)
{
}

KeyboardShortcut::KeyboardShortcut(const String& str)
{
    Vector<String> tokens;
    Split(str, "+", tokens);

#if defined(__DAVAENGINE_COREV2__)
    for (const String& token : tokens)
    {
        String t = StringUtils::Trim(token);
        int modifier = 0;
        if (GlobalEnumMap<eModifierKeys>::Instance()->ToValue(token.c_str(), modifier))
        {
            modifiers |= static_cast<eModifierKeys>(modifier);
        }
        else
        {
            DVASSERT(key == Key::UNKNOWN);
            key = InputSystem::Instance()->GetKeyboard().GetKeyByName(t);
        }
    }
#else
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
#endif

    DVASSERT(key != Key::UNKNOWN);
}

KeyboardShortcut::~KeyboardShortcut() = default;

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

#if defined(__DAVAENGINE_COREV2__)
eModifierKeys KeyboardShortcut::GetModifiers() const
#else
uint32 KeyboardShortcut::GetModifiers() const
#endif
{
    return modifiers;
}

String KeyboardShortcut::ToString() const
{
    StringStream stream;

#if defined(__DAVAENGINE_COREV2__)
    int test = static_cast<int>(eModifierKeys::FIRST);
    while (test <= static_cast<int>(eModifierKeys::LAST))
    {
        if (test & static_cast<int>(modifiers))
        {
            stream << GlobalEnumMap<eModifierKeys>::Instance()->ToString(test) << "+";
        }
        test <<= 1;
    }
#else
    int test = 0x01;
    while (test <= UIEvent::Modifier::LAST)
    {
        if (test & modifiers)
        {
            stream << GlobalEnumMap<UIEvent::Modifier>::Instance()->ToString(test) << "+";
        }
        test <<= 1;
    }
#endif
    stream << InputSystem::Instance()->GetKeyboard().GetKeyName(key);

    return stream.str();
}
}
