#include "UICustomUpdateDeltaComponent.h"

namespace DAVA
{
UIComponent* UICustomUpdateDeltaComponent::Clone() const
{
    return new UICustomUpdateDeltaComponent(*this);
}

void UICustomUpdateDeltaComponent::SetDelta(float32 delta)
{
    customDelta = delta;
}
}