#include "UIUpdateComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIUpdateComponent)
{
    ReflectionRegistrator<UIUpdateComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIUpdateComponent* c) { SafeRelease(c); })
    .Field("updateInvisible", &UIUpdateComponent::GetUpdateInvisible, &UIUpdateComponent::SetUpdateInvisible)
    .End();
}

UIUpdateComponent::UIUpdateComponent() = default;
UIUpdateComponent::~UIUpdateComponent() = default;

UIUpdateComponent::UIUpdateComponent(const UIUpdateComponent& src)
    : updateInvisible(src.updateInvisible)
{
}

UIComponent* UIUpdateComponent::Clone() const
{
    return new UIUpdateComponent(*this);
}
}