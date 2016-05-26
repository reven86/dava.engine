#ifndef __DAVAENGINE_UI_KEY_INPUT_SYSTEM_H__
#define __DAVAENGINE_UI_KEY_INPUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIEvent;
class UIFocusSystem;

class UIKeyInputSystem
{
public:
    UIKeyInputSystem(UIFocusSystem* focusSystem);
    ~UIKeyInputSystem();

    void HandleKeyEvent(UIEvent* event);

private:
    UIFocusSystem* focusSystem = nullptr;
};
}


#endif //__DAVAENGINE_UI_KEY_INPUT_SYSTEM_H__
