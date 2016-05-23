#ifndef __DAVAENGINE_DIRECTION_BASED_NAVIGATION_ALGORITHM_H__
#define __DAVAENGINE_DIRECTION_BASED_NAVIGATION_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "FocusHelpers.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIEvent;

class DirectionBasedNavigationAlgorithm
{
public:
    DirectionBasedNavigationAlgorithm(UIControl* root);
    ~DirectionBasedNavigationAlgorithm();

    UIControl* GetNextControl(UIControl* focusedControl, FocusHelpers::Direction dir);

private:
    UIControl* FindFirstControl(UIControl* control) const;
    UIControl* FindNextControl(UIControl* focusedControl, FocusHelpers::Direction dir) const;
    UIControl* FindNextSpecifiedControl(UIControl* focusedControl, FocusHelpers::Direction dir) const;
    UIControl* FindNearestControl(UIControl* focusedControl, UIControl* control, FocusHelpers::Direction dir) const;
    Vector2 CalcNearestPos(const Vector2& pos, UIControl* testControl, FocusHelpers::Direction dir) const;

    bool CanNavigateToControl(UIControl* focusedControl, UIControl* control, FocusHelpers::Direction dir) const;
    UIControl* FindFirstControlImpl(UIControl* control, UIControl* candidate) const;

    RefPtr<UIControl> root;
};
}


#endif //__DAVAENGINE_DIRECTION_BASED_NAVIGATION_ALGORITHM_H__
