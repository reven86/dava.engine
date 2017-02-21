#include "UISoundValueFilterComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UISoundValueFilterComponent::UISoundValueFilterComponent()
{
}

UISoundValueFilterComponent::UISoundValueFilterComponent(const UISoundValueFilterComponent& src)
    : step(src.step)
    , deadZone(src.deadZone)
    , normalizedValue(src.normalizedValue)
{
}

UISoundValueFilterComponent* UISoundValueFilterComponent::Clone() const
{
    return new UISoundValueFilterComponent(*this);
}

float32 UISoundValueFilterComponent::GetStep() const
{
    return step;
}

void UISoundValueFilterComponent::SetStep(float32 step_)
{
    step = step_;
}

float32 UISoundValueFilterComponent::GetDeadZone() const
{
    return deadZone;
}

void UISoundValueFilterComponent::SetDeadZone(float32 deadZone_)
{
    deadZone = deadZone_;
}
}
