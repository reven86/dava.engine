#include "UIUpdateComponent.h"

namespace DAVA
{
UIComponent* UIUpdateComponent::Clone() const
{
    return new UIUpdateComponent(*this);
}

void UIUpdateComponent::SetFunction(const UpdateFunction& f)
{
    updateFunc = f;
}

const UIUpdateComponent::UpdateFunction& UIUpdateComponent::GetFunction() const
{
    return updateFunc;
}
}