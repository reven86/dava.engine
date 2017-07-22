#include "UICustomUpdateDeltaComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UICustomUpdateDeltaComponent)
{
    ReflectionRegistrator<UICustomUpdateDeltaComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UICustomUpdateDeltaComponent* c) { SafeRelease(c); })
    .Field("delta", &UICustomUpdateDeltaComponent::GetDelta, &UICustomUpdateDeltaComponent::SetDelta)
    .End();
}

UICustomUpdateDeltaComponent::UICustomUpdateDeltaComponent() = default;
UICustomUpdateDeltaComponent::~UICustomUpdateDeltaComponent() = default;

UICustomUpdateDeltaComponent::UICustomUpdateDeltaComponent(const UICustomUpdateDeltaComponent& src)
    : customDelta(src.customDelta)
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