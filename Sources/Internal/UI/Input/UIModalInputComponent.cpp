#include "UI/Input/UIModalInputComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIModalInputComponent)
{
    ReflectionRegistrator<UIModalInputComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIModalInputComponent* o) { o->Release(); })
    .Field("enabled", &UIModalInputComponent::IsEnabled, &UIModalInputComponent::SetEnabled)
    .End();
}

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
