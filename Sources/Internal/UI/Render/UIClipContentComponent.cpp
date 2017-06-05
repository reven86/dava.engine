#include "UIClipContentComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIClipContentComponent)
{
    ReflectionRegistrator<UIClipContentComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIClipContentComponent* c) { SafeRelease(c); })
    .End();
}

UIClipContentComponent::UIClipContentComponent()
{
}

UIClipContentComponent::UIClipContentComponent(const UIClipContentComponent& src)
    : UIComponent(src)
{
}

UIClipContentComponent* UIClipContentComponent::Clone() const
{
    return new UIClipContentComponent(*this);
}
}
