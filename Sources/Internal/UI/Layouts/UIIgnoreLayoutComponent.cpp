#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "Reflection/ReflectionRegistrator.h"

#include "Math/Vector.h"
#include "UI/UIControl.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIIgnoreLayoutComponent)
{
    ReflectionRegistrator<UIIgnoreLayoutComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIIgnoreLayoutComponent* o) { o->Release(); })
    .Field("enabled", &UIIgnoreLayoutComponent::IsEnabled, &UIIgnoreLayoutComponent::SetEnabled)
    .End();
}

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

    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
