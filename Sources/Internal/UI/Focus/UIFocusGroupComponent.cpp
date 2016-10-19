#include "UIFocusGroupComponent.h"

namespace DAVA
{
DAVA_REFLECTION_IMPL(UIFocusGroupComponent)
{
    ReflectionRegistrator<UIFocusGroupComponent>::Begin()
    .End();
}

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
