#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Functional/Function.h"

namespace DAVA
{
class UIUpdateComponent : public UIBaseComponent<UIComponent::UPDATE_COMPONENT>
{
public:
    UIUpdateComponent() = default;
    UIUpdateComponent(const UIUpdateComponent& src) = default;
    UIComponent* Clone() const override;

protected:
    ~UIUpdateComponent() override = default;
};
}