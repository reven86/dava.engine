#include "UIFocusComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UIFocusComponent::UIFocusComponent()
{
}

UIFocusComponent::UIFocusComponent(const UIFocusComponent& src)
    : enabled(src.enabled)
    , requestFocus(src.requestFocus)
    , policy(src.policy)
    , leftControl(src.leftControl)
    , rightControl(src.rightControl)
    , upControl(src.upControl)
    , downControl(src.downControl)
    , tabOrder(src.tabOrder)
{
}

UIFocusComponent::~UIFocusComponent()
{
}

UIFocusComponent* UIFocusComponent::Clone() const
{
    return new UIFocusComponent(*this);
}

bool UIFocusComponent::IsEnabled() const
{
    return enabled;
}

void UIFocusComponent::SetEnabled(bool value)
{
    enabled = value;
}

bool UIFocusComponent::IsRequestFocus() const
{
    return requestFocus;
}

void UIFocusComponent::SetRequestFocus(bool value)
{
    requestFocus = value;
}

UIFocusComponent::ePolicy UIFocusComponent::GetPolicy() const
{
    return policy;
}

void UIFocusComponent::SetPolicy(ePolicy policy_)
{
    policy = policy_;
}

const String& UIFocusComponent::GetLeft() const
{
    return leftControl;
}

void UIFocusComponent::SetLeft(const String& val)
{
    leftControl = val;
}

const String& UIFocusComponent::GetRight() const
{
    return rightControl;
}

void UIFocusComponent::SetRight(const String& val)
{
    rightControl = val;
}

const String& UIFocusComponent::GetUp() const
{
    return upControl;
}

void UIFocusComponent::SetUp(const String& val)
{
    upControl = val;
}

const String& UIFocusComponent::GetDown() const
{
    return downControl;
}

void UIFocusComponent::SetDown(const String& val)
{
    downControl = val;
}

const String& UIFocusComponent::GetControlInDirection(FocusHelpers::Direction dir)
{
    switch (dir)
    {
    case FocusHelpers::LEFT:
        return leftControl;

    case FocusHelpers::RIGHT:
        return rightControl;

    case FocusHelpers::UP:
        return upControl;

    case FocusHelpers::DOWN:
        return downControl;

    default:
        DVASSERT(false);
        return leftControl;
    }
}

int32 UIFocusComponent::GetTabOrder() const
{
    return tabOrder;
}

void UIFocusComponent::SetTabOrder(int32 val)
{
    tabOrder = val;
}

int32 UIFocusComponent::GetPolicyAsInt() const
{
    return static_cast<int32>(policy);
}

void UIFocusComponent::SetPolicyFromInt(int32 policy)
{
    SetPolicy(static_cast<ePolicy>(policy));
}
}
