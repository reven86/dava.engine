#ifndef __DAVAENGINE_UI_FOCUS_SYSTEM_H__
#define __DAVAENGINE_UI_FOCUS_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "FocusHelpers.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIEvent;

class UIFocusSystem
{
public:
    UIFocusSystem();
    ~UIFocusSystem();

    UIControl* GetRoot() const;
    void SetRoot(UIControl* control);

    UIControl* GetFocusedControl() const;
    void SetFocusedControl(UIControl* control);

    void ControlBecomInvisible(UIControl* control);

    bool MoveFocusLeft();
    bool MoveFocusRight();
    bool MoveFocusUp();
    bool MoveFocusDown();

    bool MoveFocusForward();
    bool MoveFocusBackward();

private:
    bool MoveFocus(FocusHelpers::Direction dir);
    bool MoveFocus(FocusHelpers::TabDirection dir);

    void ClearFocusState(UIControl* control);
    UIControl* FindFirstControl(UIControl* control) const;
    bool IsControlBetterForFocusThanCandidate(UIControl* c1, UIControl* c2) const;

    RefPtr<UIControl> focusedControl;
    RefPtr<UIControl> root;
};
}


#endif //__DAVAENGINE_UI_FOCUS_SYSTEM_H__
