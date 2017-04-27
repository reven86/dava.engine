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
class UICustomUpdateDeltaComponent : public UIBaseComponent<UICustomUpdateDeltaComponent>
{
    DAVA_VIRTUAL_REFLECTION(UICustomUpdateDeltaComponent, UIComponent);

public:
    UICustomUpdateDeltaComponent();
    UICustomUpdateDeltaComponent(const UICustomUpdateDeltaComponent& src);
    UIComponent* Clone() const override;

    void SetDelta(float32 delta);
    float32 GetDelta() const;

protected:
    ~UICustomUpdateDeltaComponent() override;

private:
    float32 customDelta = 0.f;
};

inline float32 UICustomUpdateDeltaComponent::GetDelta() const
{
    return customDelta;
}
}