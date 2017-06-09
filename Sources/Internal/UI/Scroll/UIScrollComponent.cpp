#include "UIScrollComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIScrollComponent)
{
    ReflectionRegistrator<UIScrollComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIScrollComponent* o) { o->Release(); })
    .End();
}

UIScrollComponent::UIScrollComponent()
{
}

UIScrollComponent::UIScrollComponent(const UIScrollComponent& src)
{
}

UIScrollComponent::~UIScrollComponent()
{
}

UIScrollComponent* UIScrollComponent::Clone() const
{
    return new UIScrollComponent(*this);
}
}
