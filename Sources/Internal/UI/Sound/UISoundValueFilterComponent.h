#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISoundValueFilterComponent : public UIBaseComponent<UIComponent::SOUND_VALUE_FILTER_COMPONENT>
{
public:
    UISoundValueFilterComponent();
    UISoundValueFilterComponent(const UISoundValueFilterComponent& src);

    UISoundValueFilterComponent* Clone() const override;

    float32 GetStep() const;
    void SetStep(float32 step);

protected:
    ~UISoundValueFilterComponent() override = default;

private:
    friend class UISoundSystem;

    UISoundValueFilterComponent& operator=(const UISoundValueFilterComponent&) = delete;

    float32 step = 1.0f;
    int32 normalizedValue = -1;

public:
    INTROSPECTION_EXTEND(UISoundValueFilterComponent, UIComponent,
                         PROPERTY("step", "Step", GetStep, SetStep, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         );
};
}
