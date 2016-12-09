#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Functional/Function.h"

namespace DAVA
{
/**
Component-marker for UIUpdateSystem.
Temporary component for backward compatibility with existing code.
**WILL BE REMOVED** after refactoring all `UIControl::Update` logic.
*/
class UIUpdateComponent : public UIBaseComponent<UIComponent::UPDATE_COMPONENT>
{
public:
    UIUpdateComponent() = default;
    UIUpdateComponent(const UIUpdateComponent& src) = default;
    UIComponent* Clone() const override;

    void SetUpdateInvisible(bool value);
    bool GetUpdateInvisible() const;

protected:
    ~UIUpdateComponent() override;

private:
    bool updateInvisible = false;
};

inline void UIUpdateComponent::SetUpdateInvisible(bool value)
{
    updateInvisible = value;
}

inline bool DAVA::UIUpdateComponent::GetUpdateInvisible() const
{
    return updateInvisible;
}
}