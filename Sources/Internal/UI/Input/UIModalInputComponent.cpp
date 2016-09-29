#include "UIModalInputComponent.h"

namespace DAVA
{
UIModalInputComponent::UIModalInputComponent()
{
}

UIModalInputComponent::UIModalInputComponent(const UIModalInputComponent& src)
    : enabled(src.enabled)
{
}

UIModalInputComponent::~UIModalInputComponent()
{
}

UIModalInputComponent* UIModalInputComponent::Clone() const
{
    return new UIModalInputComponent(*this);
}

bool UIModalInputComponent::IsEnabled() const
{
    return enabled;
}

void UIModalInputComponent::SetEnabled(bool enabled_)
{
    enabled = enabled_;
}
}
