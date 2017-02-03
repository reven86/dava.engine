#pragma once

#include "Base/BaseTypes.h"
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

private:
    static const DAVA::FastName SOUND_PARAM_PAN;
    static const DAVA::FastName UI_SOUND_GROUP;

    using SoundEventMap = UnorderedMap<FastName, RefPtr<SoundEvent>>;

    void TriggerEvent(const FastName& eventName, const UIEvent* uiEvent, UIControl* control);
    RefPtr<SoundEvent> GetEvent(const FastName& eventName);

    SoundEventMap soundEvents;
};
}
