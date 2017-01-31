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

    //     enum eEventType
    //     {
    //         EVENT_TOUCH_DOWN = 1, //!<Trigger when mouse button or touch comes down inside the control.
    //         EVENT_TOUCH_UP_INSIDE = 2, //!<Trigger when mouse pressure or touch processed by the control is released.
    //         EVENT_VALUE_CHANGED = 3, //!<Used with sliders, spinners and switches. Trigger when value of the control is changed. Non-NULL callerData means that value is changed from code, not from UI.
    //         EVENT_HOVERED_SET = 4, //!<
    //         EVENT_HOVERED_REMOVED = 5, //!<
    //         EVENT_FOCUS_SET = 6, //!<Trigger when control becomes focused
    //         EVENT_FOCUS_LOST = 7, //!<Trigger when control losts focus
    //         EVENT_TOUCH_UP_OUTSIDE = 8, //!<Trigger when mouse pressure or touch processed by the control is released outside of the control.
    //         EVENT_ALL_ANIMATIONS_FINISHED = 9, //!<Trigger when all animations associated with control are ended.
    //         EVENTS_COUNT
    //     };

public:
    INTROSPECTION_EXTEND(UISoundComponent, UIComponent,
                         PROPERTY("touchDownEventName", "EVENT_TOUCH_DOWN", GetTouchDownSoundEventName, SetTouchDownSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("touchUpInsideEventName", "EVENT_TOUCH_UP_INSIDE", GetTouchUpInsideSoundEventName, SetTouchUpInsideSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         PROPERTY("valueChangedEventName", "EVENT_VALUE_CHANGED", GetValueChangedSoundEventName, SetValueChangedSoundEventName, I_SAVE | I_VIEW | I_EDIT | I_LOAD)
                         );
};
}
