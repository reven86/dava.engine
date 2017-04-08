#include "UIFocusComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIFocusComponent)
{
    ReflectionRegistrator<UIFocusComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIFocusComponent* o) { o->Release(); })
    .Field("enabled", &UIFocusComponent::IsEnabled, &UIFocusComponent::SetEnabled)
    .Field("requestFocus", &UIFocusComponent::IsRequestFocus, &UIFocusComponent::SetRequestFocus)
    .End();
}

UIFocusComponent::UIFocusComponent()
{
}

UIFocusComponent::UIFocusComponent(const UIFocusComponent& src)
    : enabled(src.enabled)
    , requestFocus(src.requestFocus)
{
}

UIFocusComponent::~UIFocusComponent()
{
}

UIFocusComponent* UIFocusComponent::Clone() const
{
    return new UIFocusComponent(*this);
}

bool UIFocusComponent::IsEnabled() const
{
    return enabled;
}

void UIFocusComponent::SetEnabled(bool value)
{
    enabled = value;
}

bool UIFocusComponent::IsRequestFocus() const
{
    return requestFocus;
}

void UIFocusComponent::SetRequestFocus(bool value)
{
    requestFocus = value;
}
}
