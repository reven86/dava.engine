#include "UI/UIAnalitycs.h"

#include "Core/Core.h"
#include "Platform/DateTime.h"
#include "Input/KeyboardDevice.h"
#include "UI/UIEvent.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace Analytics
{
bool EmitUIEvent(UIControl* control, UIControl::eEventType eventType, UIEvent* uiEvent)
{
    // Process only touch up
    if (eventType != UIControl::EVENT_TOUCH_UP_INSIDE)
    {
        return false;
    }

    // Check if core is not ready
    Analytics::Core& core = DAVA::Core::Instance()->GetAnalyticsCore();
    if (!core.IsStarted())
    {
        return false;
    }

    // Create event record
    const char* eventName = uiEvent->tapCount == 1 ? clickEvent : doubleClickEvent;
    EventRecord event(uiEventCategory, eventName);
    event.fields[controlNameTag] = GetUIControlName(control);

    // Process
    return core.PostEvent(event);
}

bool EmitKeyEvent(UIControl* control, UIEvent* uiEvent)
{
    if (uiEvent->phase != UIEvent::Phase::KEY_DOWN)
    {
        return false;
    }
    const char* pressedKey = nullptr;

    if (uiEvent->key == Key::ESCAPE)
    {
        pressedKey = escKeyPressed;
    }
    else if (uiEvent->key == Key::ENTER)
    {
        pressedKey = enterKeyPressed;
    }
    else if (uiEvent->key == Key::BACK)
    {
        pressedKey = backKeyPressed;
    }
    else
    {
        return false;
    }

    // Check if core is not ready
    Analytics::Core& core = DAVA::Core::Instance()->GetAnalyticsCore();
    if (!core.IsStarted())
    {
        return false;
    }

    // Create event record
    EventRecord event(uiEventCategory, keyPressEvent);
    event.fields[controlNameTag] = GetUIControlName(control);
    event.fields[pressedKeyTag] = pressedKey;

    // Process
    return core.PostEvent(event);
}

bool IsUIEvent(const EventRecord& record)
{
    return record.GetField(eventCategoryTag)->Cast<String>() == uiEventCategory;
}

String GetUIControlName(UIControl* uiControl)
{
    using StringDefPair = std::pair<const char*, size_t>;
    Deque<StringDefPair> controlPath;
    size_t wholeLen = 0;

    const UIControl* control = uiControl;
    while (control != nullptr)
    {
        const char* name = control->GetName().c_str();
        if (name == nullptr)
        {
            name = "";
        }

        size_t len = ::strlen(name);
        wholeLen += len;
        controlPath.emplace_front(name, len);
        control = control->GetParent();
    }

    String controlName;
    controlName.reserve(wholeLen + controlPath.size() * 2 - 1);
    controlName.append(controlPath[0].first, controlPath[0].second);

    for (auto i = controlPath.begin() + 1; i != controlPath.end(); ++i)
    {
        controlName += '/';
        controlName.append(i->first, i->second);
    }

    return controlName;
}

} // namespace Analytics
} // namespace DAVA