#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"
#include "Functional/Function.h"

namespace DAVA
{
/**
Component for UIUpdateSystem that defines which frameDelta will be send to UIControl.
Temporary component for backward compatibility with existing code.
**WILL BE CHANGED** after design replays/custom speed logic.
*/
class UICustomUpdateDeltaComponent : public UIBaseComponent<UIComponent::CUSTOM_UPDATE_DELTA_COMPONENT>
{
public:
    UICustomUpdateDeltaComponent() = default;
    UICustomUpdateDeltaComponent(const UICustomUpdateDeltaComponent& src) = default;
    UIComponent* Clone() const override;

    void SetDelta(float32 delta);
    float32 GetDelta() const;

protected:
    ~UICustomUpdateDeltaComponent() override;

private:
    float32 customDelta = 0.f;

public:
    INTROSPECTION_EXTEND(UICustomUpdateDeltaComponent, UIComponent,
                         PROPERTY("delta", "Delta", GetDelta, SetDelta, I_SAVE | I_VIEW | I_EDIT))
};

inline float32 UICustomUpdateDeltaComponent::GetDelta() const
{
    return customDelta;
}
}