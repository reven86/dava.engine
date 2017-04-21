#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISoundValueFilterComponent : public UIBaseComponent<UIComponent::SOUND_VALUE_FILTER_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UISoundValueFilterComponent, UIBaseComponent<UIComponent::SOUND_VALUE_FILTER_COMPONENT>);

public:
    UISoundValueFilterComponent();
    UISoundValueFilterComponent(const UISoundValueFilterComponent& src);

    UISoundValueFilterComponent* Clone() const override;

    float32 GetStep() const;
    void SetStep(float32 step);

    float32 GetDeadZone() const;
    void SetDeadZone(float32 deadZone);

protected:
    ~UISoundValueFilterComponent() override = default;

private:
    friend class UISoundSystem;

    UISoundValueFilterComponent& operator=(const UISoundValueFilterComponent&) = delete;

    float32 step = 1.0f;
    float32 deadZone = 0.0f;
    int32 normalizedValue = -1;
};
}
