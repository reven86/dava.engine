#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Functional/Function.h"

namespace DAVA
{
class UICustomUpdateDeltaComponent : public UIBaseComponent<UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT>
{
public:
    UICustomUpdateDeltaComponent() = default;
    UICustomUpdateDeltaComponent(const UICustomUpdateDeltaComponent& src) = default;
    UIComponent* Clone() const override;

    void SetDelta(float32 delta);
    float32 GetDelta() const;

protected:
    ~UICustomUpdateDeltaComponent() override = default;

private:
    float32 customDelta = 0.f;
};

inline float32 UICustomUpdateDeltaComponent::GetDelta() const
{
    return customDelta;
}
}