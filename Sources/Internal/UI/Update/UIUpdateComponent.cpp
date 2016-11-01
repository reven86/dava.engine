#include "UIUpdateComponent.h"

namespace DAVA
{
UIUpdateComponent::UIUpdateComponent(const UIUpdateComponent& src)
    : updateFunc(src.updateFunc)
{
}

UIUpdateComponent::UIUpdateComponent(const UpdateFunction& f)
    : updateFunc(f)
{
}

UIComponent* UIUpdateComponent::Clone() const
{
    return new UIUpdateComponent(*this);
}

void UIUpdateComponent::SetUpdateFunction(const UpdateFunction& f)
{
    updateFunc = f;
}

const UIUpdateComponent::UpdateFunction& UIUpdateComponent::GetUpdateFunction() const
{
    return updateFunc;
}
}