#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Functional/Function.h"

namespace DAVA
{
class UIUpdateComponent : public UIBaseComponent<UIComponent::UPDATE_COMPONENT>
{
public:
    using UpdateFunction = Function<void(float32)>;

    UIUpdateComponent() = default;
    UIUpdateComponent(const UIUpdateComponent& src) = default;
    UIComponent* Clone() const override;

    void SetFunction(const UpdateFunction& f);
    const UpdateFunction& GetFunction() const;

protected:
    ~UIUpdateComponent() override = default;

private:
    UpdateFunction updateFunc;
};
}