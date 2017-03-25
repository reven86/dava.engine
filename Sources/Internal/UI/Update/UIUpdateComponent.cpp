#include "UIUpdateComponent.h"

namespace DAVA
{
UIUpdateComponent::~UIUpdateComponent()
{
}

UIComponent* UIUpdateComponent::Clone() const
{
    return new UIUpdateComponent(*this);
}
}