#pragma once

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UISoundComponent : public UIBaseComponent<UIComponent::SOUND_COMPONENT>
{
public:
    UISoundComponent();
    UISoundComponent(const UISoundComponent& src);

    virtual UISoundComponent* Clone() const override;

    void SetSoundEventName(UIControl::eEventType eventType, const FastName& eventName);
    const FastName& GetSoundEventName(UIControl::eEventType eventType) const;

    void SetTouchDownSoundEventName(const FastName& eventName);
    const FastName& GetTouchDownSoundEventName() const;

    void SetTouchUpInsideSoundEventName(const FastName& eventName);
    const FastName& GetTouchUpInsideSoundEventName() const;

    void SetValueChangedSoundEventName(const FastName& eventName);
    const FastName& GetValueChangedSoundEventName() const;

protected:
    virtual ~UISoundComponent();

private:
    UISoundComponent& operator=(const UISoundComponent&) = delete;

    DAVA::Array<FastName, UIControl::EVENTS_COUNT> soundEventNames;

public:
    INTROSPECTION_EXTEND(UISoundComponent, UIComponent,
                         PROPERTY("touchDownEventName", "EVENT_TOUCH_DOWN", GetTouchDownSoundEventName, SetTouchDownSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("touchUpInsideEventName", "EVENT_TOUCH_UP_INSIDE", GetTouchUpInsideSoundEventName, SetTouchUpInsideSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("valueChangedEventName", "EVENT_VALUE_CHANGED", GetValueChangedSoundEventName, SetValueChangedSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         );
};
}
