#include "UISoundValueFilterComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include "UI/UIControl.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UISoundValueFilterComponent)
{
    ReflectionRegistrator<UISoundValueFilterComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UISoundValueFilterComponent* c) { SafeRelease(c); })
    .Field("step", &UISoundValueFilterComponent::GetStep, &UISoundValueFilterComponent::SetStep)
    .Field("deadZone", &UISoundValueFilterComponent::GetDeadZone, &UISoundValueFilterComponent::SetDeadZone)
    .End();
}

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
