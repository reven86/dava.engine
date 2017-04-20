#include "UIRichContentItemComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIRichContentItemComponent)
{
    ReflectionRegistrator<UIRichContentItemComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIRichContentItemComponent* o) { o->Release(); })
    .End();
}

UIRichContentItemComponent::UIRichContentItemComponent(const UIRichContentItemComponent& src)
    : UIBaseComponent(src)
{
}

UIRichContentItemComponent* UIRichContentItemComponent::Clone() const
{
    return new UIRichContentItemComponent(*this);
}
}
