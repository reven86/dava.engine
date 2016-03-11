#include "UIActionBindingComponent.h"

#include "Utils/Utils.h"

namespace DAVA
{
UIActionBindingComponent::UIActionBindingComponent()
{
    GetActionMap().Put(FastName("BATTLE"), []() {
        Logger::Debug("BATTLE");
    });

    GetActionMap().Put(FastName("ESC"), []() {
        Logger::Debug("Escape");
    });
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
        stream << action.action.c_str() << ", " << action.shortcut1.ToString();
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
        if (str.size() > 1 && !str[0].empty())
        {
            FastName actionName(Trim(str[0]));
            KeyboardShortcut shortcut(Trim(str[1]));
            actions.push_back(Action(actionName, shortcut));
            if (shortcut.GetKey() != Key::UNKNOWN)
                inputMap.BindAction(shortcut, actionName);
        }
    }
}
}
