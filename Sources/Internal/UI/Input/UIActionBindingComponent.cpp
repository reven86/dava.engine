#include "UIActionBindingComponent.h"

#include "Utils/Utils.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIActionBindingComponent)
{
    ReflectionRegistrator<UIActionBindingComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIActionBindingComponent* o) { o->Release(); })
    .Field("actions", &UIActionBindingComponent::GetActionsAsString, &UIActionBindingComponent::SetActionsFromString)
    .Field("blockOtherShortcuts", &UIActionBindingComponent::IsBlockOtherKeyboardShortcuts, &UIActionBindingComponent::SetBlockOtherKeyboardShortcuts)
    .End();
}

UIActionBindingComponent::UIActionBindingComponent()
{
}

UIActionBindingComponent::UIActionBindingComponent(const UIActionBindingComponent& src)
{
}

UIActionBindingComponent::~UIActionBindingComponent()
{
}

UIActionBindingComponent* UIActionBindingComponent::Clone() const
{
    return new UIActionBindingComponent(*this);
}

const UIActionMap& UIActionBindingComponent::GetActionMap() const
{
    return actionMap;
}

UIActionMap& UIActionBindingComponent::GetActionMap()
{
    return actionMap;
}

const UIInputMap& UIActionBindingComponent::GetInputMap() const
{
    return inputMap;
}

UIInputMap& UIActionBindingComponent::GetInputMap()
{
    return inputMap;
}

bool UIActionBindingComponent::IsBlockOtherKeyboardShortcuts() const
{
    return blockOtherKeyboardShortcuts;
}

void UIActionBindingComponent::SetBlockOtherKeyboardShortcuts(bool block)
{
    blockOtherKeyboardShortcuts = block;
}

String UIActionBindingComponent::GetActionsAsString() const
{
    StringStream stream;
    bool first = true;
    for (const Action& action : actions)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << "; ";
        }

        if (action.shortcut1.GetKey() != Key::UNKNOWN)
        {
            stream << action.action.c_str() << ", " << action.shortcut1.ToString();
        }
        else
        {
            stream << action.action.c_str();
        }
    }
    return stream.str();
}

void UIActionBindingComponent::SetActionsFromString(const String& value)
{
    actions.clear();
    inputMap.Clear();
    Vector<String> actionsStr;
    Split(value, ";", actionsStr);
    for (const String& actionStr : actionsStr)
    {
        Vector<String> str;
        Split(actionStr, ",", str);
        if (str.size() > 0 && !str[0].empty())
        {
            FastName actionName(StringUtils::Trim(str[0]));
            KeyboardShortcut shortcut;

            if (str.size() > 1)
            {
                shortcut = KeyboardShortcut(StringUtils::Trim(str[1]));
            }

            actions.push_back(Action(actionName, shortcut));
            if (shortcut.GetKey() != Key::UNKNOWN)
                inputMap.BindAction(shortcut, actionName);
        }
    }
}
}
