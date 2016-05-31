#include "UIFocusGroupComponent.h"

namespace DAVA
{
UIFocusGroupComponent::UIFocusGroupComponent()
{
}

UIFocusGroupComponent::UIFocusGroupComponent(const UIFocusGroupComponent& src)
{
}

UIFocusGroupComponent::~UIFocusGroupComponent()
{
}

UIFocusGroupComponent* UIFocusGroupComponent::Clone() const
{
    return new UIFocusGroupComponent(*this);
}
}
