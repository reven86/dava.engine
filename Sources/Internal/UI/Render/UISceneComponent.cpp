#include "UISceneComponent.h"

#include "UI/UIControl.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UISceneComponent)
{
    ReflectionRegistrator<UISceneComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UISceneComponent* o) { o->Release(); })
    .End();
}

UISceneComponent::UISceneComponent() = default;

UISceneComponent::~UISceneComponent() = default;

UISceneComponent::UISceneComponent(const UISceneComponent& src)
    : UIBaseComponent(src)
{
}

UISceneComponent* UISceneComponent::Clone() const
{
    return new UISceneComponent(*this);
}
}
