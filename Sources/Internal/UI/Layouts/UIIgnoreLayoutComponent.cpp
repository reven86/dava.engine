#include "UIIgnoreLayoutComponent.h"

#include "Math/Vector.h"

namespace DAVA
{
UIIgnoreLayoutComponent::UIIgnoreLayoutComponent(const UIIgnoreLayoutComponent& src)
    : enabled(src.enabled)
{
}

UIIgnoreLayoutComponent* UIIgnoreLayoutComponent::Clone() const
{
    return new UIIgnoreLayoutComponent(*this);
}

bool UIIgnoreLayoutComponent::IsEnabled() const
{
    return enabled;
}

void UIIgnoreLayoutComponent::SetEnabled(bool enabled_)
{
    enabled = enabled_;
}
}
