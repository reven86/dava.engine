#pragma once

#include "Base/BaseTypes.h"
#include "Sound/SoundEvent.h"
#include "UI/UIControl.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;

class UISoundSystem
: public UISystem
{
public:
    UISoundSystem();
    ~UISoundSystem() override;

    void ProcessControlEvent(int32 eventType, const UIEvent* uiEvent, UIControl* control);
    void FreeEvents();

    void SetGlobalParameter(const DAVA::FastName& parameter, float value);

private:
    static const DAVA::FastName SOUND_PARAM_PAN;
    static const DAVA::FastName SOUND_GROUP;

    using SoundEventMap = UnorderedMap<FastName, RefPtr<SoundEvent>>;
    using GlobalParameterMap = UnorderedMap<FastName, float32>;

    void TriggerEvent(const FastName& eventName, const UIEvent* uiEvent, UIControl* control);
    void SetupEventPan(SoundEvent* event, const UIEvent* uiEvent, UIControl* control);
    void SetupEventGlobalParameters(SoundEvent* event);

    RefPtr<SoundEvent> GetEvent(const FastName& eventName);

    bool ShouldSkipEvent(int32 eventType, const UIEvent* uiEvent, UIControl* control);

    GlobalParameterMap globalParameters;
    SoundEventMap soundEvents;
};
}
