#include "UISoundComponent.h"

namespace DAVA
{
UISoundComponent::UISoundComponent()
{
}

UISoundComponent::UISoundComponent(const UISoundComponent& src)
    : soundEventNames(src.soundEventNames)
{
}

UISoundComponent* UISoundComponent::Clone() const
{
    return new UISoundComponent(*this);
}

const FastName& UISoundComponent::GetSoundEventName(UIControl::eEventType uiEventType) const
{
    return soundEventNames[uiEventType];
}

void UISoundComponent::SetSoundEventName(UIControl::eEventType uiEventType, const FastName& soundEventName)
{
    soundEventNames[uiEventType] = soundEventName;
}

const FastName& UISoundComponent::GetOnTouchDownSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_TOUCH_DOWN);
}

void UISoundComponent::SetOnTouchDownSoundEventName(const FastName& soundEventName)
{
    SetSoundEventName(UIControl::EVENT_TOUCH_DOWN, soundEventName);
}

const FastName& UISoundComponent::GetOnTouchUpInsideSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_TOUCH_UP_INSIDE);
}

void UISoundComponent::SetOnTouchUpInsideSoundEventName(const FastName& soundEventName)
{
    SetSoundEventName(UIControl::EVENT_TOUCH_UP_INSIDE, soundEventName);
}

const FastName& UISoundComponent::GetOnTouchUpOutsideSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_TOUCH_UP_OUTSIDE);
}

void UISoundComponent::SetOnTouchUpOutsideSoundEventName(const FastName& soundEventName)
{
    SetSoundEventName(UIControl::EVENT_TOUCH_UP_OUTSIDE, soundEventName);
}

const FastName& UISoundComponent::GetOnValueChangedSoundEventName() const
{
    return GetSoundEventName(UIControl::EVENT_VALUE_CHANGED);
}

void UISoundComponent::SetOnValueChangedSoundEventName(const FastName& soundEventName)
{
    SetSoundEventName(UIControl::EVENT_VALUE_CHANGED, soundEventName);
}
}
