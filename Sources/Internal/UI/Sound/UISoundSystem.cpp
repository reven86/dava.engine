#include "UI/Sound/UISoundSystem.h"

#include "Engine/Engine.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/Sound/UISoundComponent.h"
#include "UI/Sound/UISoundValueFilterComponent.h"

namespace DAVA
{
const FastName UISoundSystem::SOUND_PARAM_PAN("pan");
const FastName UISoundSystem::UI_SOUND_GROUP("UI_SOUND_GROUP");

UISoundSystem::UISoundSystem()
{
}

UISoundSystem::~UISoundSystem()
{
}

void UISoundSystem::ProcessControlEvent(int32 eventType, const UIEvent* uiEvent, UIControl* control)
    {
        UISoundComponent* soundComponent = control->GetComponent<UISoundComponent>();

        if (soundComponent)
        {
            if (!ShouldSkip(eventType, uiEvent, control))
            {
                const FastName& soundEventName = soundComponent->GetSoundEventName(static_cast<UIControl::eEventType>(eventType));

                if (soundEventName.IsValid())
                {
                    TriggerEvent(soundEventName, uiEvent, control);
                }
            }
        }
    }

    void UISoundSystem::FreeEvents()
    {
        soundEvents.clear();
    }

    void UISoundSystem::SetGlobalParameter(const DAVA::FastName& parameter, float value)
    {
        globalParameters[parameter] = value;
    }

    void UISoundSystem::TriggerEvent(const FastName& eventName, const UIEvent* uiEvent, UIControl* control)
    {
        RefPtr<SoundEvent> event = GetEvent(eventName);

        if (event->IsActive())
        {
            event->Stop();
        }

        SetupEventPan(event.Get(), uiEvent, control);
        SetupEventGlobalParameters(event.Get());

        event->Trigger();
    }

    void UISoundSystem::SetupEventPan(SoundEvent* event, const UIEvent* uiEvent, UIControl* control)
    {
        if (event->IsParameterExists(SOUND_PARAM_PAN))
        {
            const float32 PAN_DEFAULT = 0.0f;
            const float32 PAN_LEFT = -1.0f;
            const float32 PAN_RIGHT = 1.0f;
            const float32 PAN_RANGE = PAN_RIGHT - PAN_LEFT;

            float32 pan = PAN_DEFAULT;
            float32 pointX = 0.0f;
            if (uiEvent)
            {
                pointX = uiEvent->point.x;
            }
            else
            {
                pointX = control->GetAbsoluteRect().GetCenter().x;
            }

#if defined(__DAVAENGINE_COREV2__)
            pan = PAN_LEFT + PAN_RANGE * (pointX / GetPrimaryWindow()->GetVirtualSize().dx);
#else
            pan = PAN_LEFT + PAN_RANGE * (pointX / VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx);
#endif

            event->SetParameterValue(SOUND_PARAM_PAN, pan);
        }
    }

    void UISoundSystem::SetupEventGlobalParameters(SoundEvent* event)
    {
        for (const GlobalParameterMap::value_type& param : globalParameters)
        {
            if (event->IsParameterExists(param.first))
            {
                event->SetParameterValue(param.first, param.second);
            }
        }
    }

    RefPtr<SoundEvent> UISoundSystem::GetEvent(const FastName& eventName)
    {
        SoundEventMap::iterator eventIter = soundEvents.find(eventName);
        if (eventIter == soundEvents.end())
        {
            RefPtr<SoundEvent> event(SoundSystem::Instance()->CreateSoundEventByID(eventName, UI_SOUND_GROUP));

            soundEvents[eventName] = event;

            return event;
        }
        else
        {
            return eventIter->second;
        }
    }

    bool UISoundSystem::ShouldSkip(int32 eventType, const UIEvent* uiEvent, UIControl* control)
    {
        if (eventType == UIControl::EVENT_VALUE_CHANGED)
        {
            UISoundValueFilterComponent* valueFilterComponent = control->GetComponent<UISoundValueFilterComponent>();
            if (valueFilterComponent)
            {
                // we need some kind of generic way to get control value, for now branch here
                UISlider* slider = dynamic_cast<UISlider*>(control);

                if (slider)
                {
                    const float32 value = slider->GetValue();

                    const int32 normalizedValue = (int32)(slider->GetValue() / valueFilterComponent->GetStep());

                    if (valueFilterComponent->normalizedValue == normalizedValue)
                    {
                        return true;
                    }

                    valueFilterComponent->normalizedValue = normalizedValue;
                }
            }
        }

        return false;
    }
}
