#include "UI/Focus/UIFocusGroupComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIFocusGroupComponent)
{
    ReflectionRegistrator<UIFocusGroupComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIFocusGroupComponent* o) { o->Release(); })
    .End();
}

UIFocusGroupComponent::UIFocusGroupComponent()
{
}

UIFocusGroupComponent::UIFocusGroupComponent(const UIFocusGroupComponent& src)
{
}

UIFocusGroupComponent::~UIFocusGroupComponent()
{
}

UIFocusGroupComponent* UIFocusGroupComponent::Clone() const
{
    return new UIFocusGroupComponent(*this);
}
}
