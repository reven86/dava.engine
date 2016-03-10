#include "UIActionBindingComponent.h"

#include "Utils/Utils.h"

namespace DAVA
{
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
            stream << ";";
        }
        stream << action.action.c_str() << ", " << action.shortcut1.ToString();
    }
    return stream.str();
}

void UIActionBindingComponent::SetActionsFromString(const String& value)
{
    actions.clear();
    Vector<String> actionsStr;
    Split(value, ";", actionsStr);
    for (const String& actionStr : actionsStr)
    {
        Vector<String> str;
        Split(actionStr, ",", str);
        if (str.size() > 1 && !str[0].empty())
        {
            actions.push_back(Action(FastName(Trim(str[0])), KeyboardShortcut(Trim(str[1]))));
        }
    }
}
}
