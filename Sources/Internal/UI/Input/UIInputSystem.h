#ifndef __DAVAENGINE_UI_INPUT_SYSTEM_H__
#define __DAVAENGINE_UI_INPUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "UI/Input/UIActionMap.h"
#include "UI/Input/UIInputMap.h"
#include "Functional/Signal.h"

namespace DAVA
{
class UIControl;
class UIScreen;
class UIEvent;
class UIFocusSystem;

class UIInputSystem
{
public:
    UIInputSystem();
    ~UIInputSystem();

    void SetCurrentScreen(UIScreen* screen);
    void SetPopupContainer(UIControl* popupContainer);

    void OnControlVisible(UIControl* control);
    void OnControlInvisible(UIControl* control);

    void HandleEvent(UIEvent* event);

    void CancelInput(UIEvent* touch);
    void CancelAllInputs();
    void CancelInputs(UIControl* control, bool hierarchical);
    void SwitchInputToControl(uint32 eventID, UIControl* targetControl);

    const Vector<UIEvent>& GetAllInputs() const;

    void SetExclusiveInputLocker(UIControl* locker, uint32 lockEventId);
    UIControl* GetExclusiveInputLocker() const;
    void SetHoveredControl(UIControl* newHovered);
    UIControl* GetHoveredControl() const;

    UIFocusSystem* GetFocusSystem() const;

    void MoveFocusLeft();
    void MoveFocusRight();
    void MoveFocusUp();
    void MoveFocusDown();

    void MoveFocusForward();
    void MoveFocusBackward();

    void BindGlobalShortcut(const KeyboardShortcut& shortcut, const FastName& actionName);
    void BindGlobalAction(const FastName& actionName, const UIActionMap::Action& action);
    void PerformActionOnControl(UIControl* control);
    void PerformActionOnFocusedControl();

    static const FastName ACTION_FOCUS_LEFT;
    static const FastName ACTION_FOCUS_RIGHT;
    static const FastName ACTION_FOCUS_UP;
    static const FastName ACTION_FOCUS_DOWN;

    static const FastName ACTION_FOCUS_NEXT;
    static const FastName ACTION_FOCUS_PREV;

    static const FastName ACTION_PERFORM;
    static const FastName ACTION_ESCAPE;

    DAVA::Signal<UIEvent*> notProcessedEventSignal;

private:
    bool HandleTouchEvent(UIEvent* event);
    bool HandleKeyEvent(UIEvent* event);
    bool HandleOtherEvent(UIEvent* event);

    void UpdateModalControl();
    void CancelInputForAllOutsideChildren(UIControl* root);

    UIControl* FindNearestToUserModalControl() const;
    UIControl* FindNearestToUserModalControlImpl(UIControl* current) const;

    UIScreen* currentScreen = nullptr;
    UIControl* popupContainer = nullptr;
    RefPtr<UIControl> modalControl;

    UIFocusSystem* focusSystem = nullptr;

    UIControl* hovered = nullptr;

    Vector<UIEvent> touchEvents;
    UIControl* focusedControlWhenTouchBegan = nullptr;
    Vector2 positionOfTouchWhenTouchBegan;
    UIControl* exclusiveInputLocker = nullptr;

    UIActionMap globalActions;
    UIInputMap globalInputMap;
    int32 modifiers = 0;
};
}


#endif //__DAVAENGINE_UI_KEY_INPUT_SYSTEM_H__
