#pragma once

#include "Analytics/Analytics.h"
#include "UI/UIControl.h"

namespace DAVA
{
namespace Analytics
{
const char uiEventTypeTag[] = "UIEventType";

const char clickEvent[] = "Click";
const char doubleClickEvent[] = "DoubleClick";
const char keyPressEvent[] = "KeyPress";

const char pressedKeyTag[] = "PressedKey";
const char escKeyPressed[] = "EscKeyPressed";
const char backKeyPressed[] = "BackKeyPressed";
const char enterKeyPressed[] = "EnterKeyPressed";

bool EmitUIEvent(UIControl* control, UIControl::eEventType eventType, UIEvent* uiEvent);
bool EmitKeyEvent(UIControl* control, UIEvent* uiEvent);
bool IsUIEvent(const EventRecord& record);
String GetUIControlName(UIControl* uiControl);

} // namespace Analytics
} // namespace DAVA