#include "UIFocusComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UIFocusComponent::UIFocusComponent()
{
}

UIFocusComponent::UIFocusComponent(const UIFocusComponent& src)
    : tabOrder(src.tabOrder)
    , policy(src.policy)
    , enabled(src.enabled)
    , requestFocus(src.requestFocus)
{
    for (int i = 0; i < FocusHelpers::DIRECTION_COUNT; i++)
    {
        nextFocusPath[i] = src.nextFocusPath[i];
    }
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

const String& UIFocusComponent::GetNextFocusLeft() const
{
    return nextFocusPath[FocusHelpers::LEFT];
}

void UIFocusComponent::SetNextFocusLeft(const String& val)
{
    nextFocusPath[FocusHelpers::LEFT] = val;
}

const String& UIFocusComponent::GetNextFocusRight() const
{
    return nextFocusPath[FocusHelpers::RIGHT];
}

void UIFocusComponent::SetNextFocusRight(const String& val)
{
    nextFocusPath[FocusHelpers::RIGHT] = val;
}

const String& UIFocusComponent::GetNextFocusUp() const
{
    return nextFocusPath[FocusHelpers::UP];
}

void UIFocusComponent::SetNextFocusUp(const String& val)
{
    nextFocusPath[FocusHelpers::UP] = val;
}

const String& UIFocusComponent::GetNextFocusDown() const
{
    return nextFocusPath[FocusHelpers::DOWN];
}

void UIFocusComponent::SetNextFocusDown(const String& val)
{
    nextFocusPath[FocusHelpers::DOWN] = val;
}

const String& UIFocusComponent::GetNextControlPathInDirection(FocusHelpers::Direction dir)
{
    DVASSERT(0 <= dir && dir < FocusHelpers::DIRECTION_COUNT);
    return nextFocusPath[dir];
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
