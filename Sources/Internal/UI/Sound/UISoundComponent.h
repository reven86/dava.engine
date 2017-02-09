#pragma once

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISoundComponent : public UIBaseComponent<UIComponent::SOUND_COMPONENT>
{
public:
    UISoundComponent();
    UISoundComponent(const UISoundComponent& src);

    virtual UISoundComponent* Clone() const override;

    void SetSoundEventName(UIControl::eEventType uiEventType, const FastName& soundEventName);
    const FastName& GetSoundEventName(UIControl::eEventType uiEventType) const;

    void SetOnTouchDownSoundEventName(const FastName& soundEventName);
    const FastName& GetOnTouchDownSoundEventName() const;

    void SetOnTouchUpInsideSoundEventName(const FastName& soundEventName);
    const FastName& GetOnTouchUpInsideSoundEventName() const;

    void SetOnTouchUpOutsideSoundEventName(const FastName& soundEventName);
    const FastName& GetOnTouchUpOutsideSoundEventName() const;

    void SetOnValueChangedSoundEventName(const FastName& soundEventName);
    const FastName& GetOnValueChangedSoundEventName() const;

protected:
    ~UISoundComponent() override = default;

private:
    UISoundComponent& operator=(const UISoundComponent&) = delete;

    DAVA::Array<FastName, UIControl::EVENTS_COUNT> soundEventNames;

public:
    INTROSPECTION_EXTEND(UISoundComponent, UIComponent,
                         PROPERTY("touchDown", "Touch Down", GetOnTouchDownSoundEventName, SetOnTouchDownSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("touchUpInside", "Touch Up Inside", GetOnTouchUpInsideSoundEventName, SetOnTouchUpInsideSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("touchUpOutside", "Touch Up Outside", GetOnTouchUpOutsideSoundEventName, SetOnTouchUpOutsideSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("valueChanged", "Value Changed", GetOnValueChangedSoundEventName, SetOnValueChangedSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD));
};
}
