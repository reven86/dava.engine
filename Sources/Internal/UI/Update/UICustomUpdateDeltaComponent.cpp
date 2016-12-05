#include "UICustomUpdateDeltaComponent.h"

namespace DAVA
{
UICustomUpdateDeltaComponent::~UICustomUpdateDeltaComponent()
{
}

UIComponent* UICustomUpdateDeltaComponent::Clone() const
{
    return new UICustomUpdateDeltaComponent(*this);
}

void UICustomUpdateDeltaComponent::SetDelta(float32 delta)
{
    customDelta = delta;
}
}