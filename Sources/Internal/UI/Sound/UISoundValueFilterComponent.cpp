#include "UISoundValueFilterComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UISoundValueFilterComponent::UISoundValueFilterComponent()
{
}

UISoundValueFilterComponent::UISoundValueFilterComponent(const UISoundValueFilterComponent& src)
    : step(src.step)
{
}

UISoundValueFilterComponent::~UISoundValueFilterComponent()
{
}

UISoundValueFilterComponent* UISoundValueFilterComponent::Clone() const
{
    return new UISoundValueFilterComponent(*this);
}

void UISoundValueFilterComponent::SetStep(float32 step_)
{
    step = step_;
}

float32 UISoundValueFilterComponent::GetStep() const
{
    return step;
}
}
