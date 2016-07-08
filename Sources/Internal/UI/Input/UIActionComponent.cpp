#include "UIActionComponent.h"

#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"

namespace DAVA
{
UIActionComponent::UIActionComponent()
{
}

UIActionComponent::UIActionComponent(const UIActionComponent& src)
    : action(src.action)
{
}

UIActionComponent::~UIActionComponent()
{
}

UIActionComponent* UIActionComponent::Clone() const
{
    return new UIActionComponent(*this);
}

const FastName& UIActionComponent::GetAction() const
{
    return action;
}

void UIActionComponent::SetAction(const FastName& value)
{
    action = value;
}

String UIActionComponent::GetActionAsString() const
{
    return GetAction().IsValid() ? GetAction().c_str() : String("");
}

void UIActionComponent::SetActionFromString(const String& value)
{
    SetAction(value.empty() ? FastName() : FastName(value));
}
}
