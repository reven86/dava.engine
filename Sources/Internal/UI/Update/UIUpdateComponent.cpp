#include "UIUpdateComponent.h"

namespace DAVA
{
UIComponent* UIUpdateComponent::Clone() const
{
    return new UIUpdateComponent(*this);
}
}