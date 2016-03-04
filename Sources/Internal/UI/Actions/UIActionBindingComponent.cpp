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
    for (const Action& action : actions)
    {
        stream << action.action.c_str() << ", " << action.shortcut1.ToString() << ";";
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
        if (str.size() > 1)
        {
            actions.push_back(Action(FastName(str[0]), KeyboardShortcut(str[1])));
        }
    }
}
}
