#include "UILayoutIsolationComponent.h"

#include "UI/UIControl.h"
#include "Math/Vector.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UILayoutIsolationComponent)
{
    ReflectionRegistrator<UILayoutIsolationComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UILayoutIsolationComponent* o) { o->Release(); })
    .End();
}
    
UILayoutIsolationComponent::UILayoutIsolationComponent()
{
}

UILayoutIsolationComponent::UILayoutIsolationComponent(const UILayoutIsolationComponent& src)
{
}

UILayoutIsolationComponent::~UILayoutIsolationComponent()
{
}

UILayoutIsolationComponent* UILayoutIsolationComponent::Clone() const
{
    return new UILayoutIsolationComponent(*this);
}
}
