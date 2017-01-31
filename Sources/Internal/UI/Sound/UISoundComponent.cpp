#include "UISoundComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UISoundComponent::UISoundComponent()
{
}

UISoundComponent::UISoundComponent(const UISoundComponent& src)
{
}

UISoundComponent::~UISoundComponent()
{
}

UISoundComponent* UISoundComponent::Clone() const
{
    return new UISoundComponent(*this);
}

void UISoundComponent::SetSoundEventName(UIControl::eEventType eventType, const FastName& eventName)
{
    soundEventNames[eventType] = eventName;
}

const FastName& UISoundComponent::GetSoundEventName(UIControl::eEventType eventType) const
{
    return soundEventNames[eventType];
}

void UISoundComponent::SetTouchDownSoundEventName(const FastName& eventName)
{
    SetSoundEventName(UIControl::EVENT_TOUCH_DOWN, eventName);
}

const FastName& UISoundComponent::GetTouchDownSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_TOUCH_DOWN);
}

void UISoundComponent::SetTouchUpInsideSoundEventName(const FastName& eventName)
{
    SetSoundEventName(UIControl::EVENT_TOUCH_UP_INSIDE, eventName);
}

const FastName& UISoundComponent::GetTouchUpInsideSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_TOUCH_UP_INSIDE);
}

void UISoundComponent::SetValueChangedSoundEventName(const FastName& eventName)
{
    SetSoundEventName(UIControl::EVENT_VALUE_CHANGED, eventName);
}

const FastName& UISoundComponent::GetValueChangedSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_VALUE_CHANGED);
}
}
