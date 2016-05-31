#include "UiModalInputComponent.h"

namespace DAVA
{
UIModalInputComponent::UIModalInputComponent()
{
}

UIModalInputComponent::UIModalInputComponent(const UIModalInputComponent& src)
{
}

UIModalInputComponent::~UIModalInputComponent()
{
}

UIModalInputComponent* UIModalInputComponent::Clone() const
{
    return new UIModalInputComponent(*this);
}
}
