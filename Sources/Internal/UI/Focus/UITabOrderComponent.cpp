#include "UITabOrderComponent.h"

namespace DAVA
{
DAVA_REFLECTION_IMPL(UITabOrderComponent)
{
    ReflectionRegistrator<UITabOrderComponent>::Begin()
    .Field("tab", &UITabOrderComponent::GetTabOrder, &UITabOrderComponent::SetTabOrder)
    .End();
}

UITabOrderComponent::UITabOrderComponent()
{
}

UITabOrderComponent::UITabOrderComponent(const UITabOrderComponent& src)
    : tabOrder(src.tabOrder)
{
}

UITabOrderComponent::~UITabOrderComponent()
{
}

UITabOrderComponent* UITabOrderComponent::Clone() const
{
    return new UITabOrderComponent(*this);
}

int32 UITabOrderComponent::GetTabOrder() const
{
    return tabOrder;
}

void UITabOrderComponent::SetTabOrder(int32 val)
{
    tabOrder = val;
}
}
