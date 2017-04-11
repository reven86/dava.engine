#include "UILayoutIsolationComponent.h"

#include "UI/UIControl.h"
#include "Math/Vector.h"

namespace DAVA
{
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
