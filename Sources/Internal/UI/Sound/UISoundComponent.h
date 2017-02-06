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
    virtual ~UISoundComponent();

private:
    UISoundComponent& operator=(const UISoundComponent&) = delete;

    DAVA::Array<FastName, UIControl::EVENTS_COUNT> soundEventNames;

public:
    INTROSPECTION_EXTEND(UISoundComponent, UIComponent,
                         PROPERTY("onTouchDownSoundEventName", "EVENT_TOUCH_DOWN", GetOnTouchDownSoundEventName, SetOnTouchDownSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("onTouchUpInsideSoundEventName", "EVENT_TOUCH_UP_INSIDE", GetOnTouchUpInsideSoundEventName, SetOnTouchUpInsideSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("onTouchUpOutsideSoundEventName", "EVENT_TOUCH_UP_OUTSIDE", GetOnTouchUpOutsideSoundEventName, SetOnTouchUpOutsideSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("onValueChangedSoundEventName", "EVENT_VALUE_CHANGED", GetOnValueChangedSoundEventName, SetOnValueChangedSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD));
};
}
