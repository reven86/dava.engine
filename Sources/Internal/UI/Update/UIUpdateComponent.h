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
    UIUpdateComponent(const UIUpdateComponent& src);
    UIUpdateComponent(const UpdateFunction& f);
    UIComponent* Clone() const override;

    void SetUpdateFunction(const UpdateFunction& f);
    const UpdateFunction& GetUpdateFunction() const;

protected:
    ~UIUpdateComponent() override = default;

private:
    UpdateFunction updateFunc;
};
}