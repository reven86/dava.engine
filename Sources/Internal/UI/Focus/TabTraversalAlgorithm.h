#ifndef __DAVAENGINE_TAB_TRAVERSAL_ALGORITHM_H__
#define __DAVAENGINE_TAB_TRAVERSAL_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "FocusHelpers.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIEvent;

class TabTraversalAlgorithm
{
public:
    TabTraversalAlgorithm(UIControl* root);
    ~TabTraversalAlgorithm();

    UIControl* GetNextControl(UIControl* focusedControl, FocusHelpers::TabDirection dir);

private:
    template <typename It>
    UIControl* FindNextControl(UIControl* focusedControl, It begin, It end, FocusHelpers::TabDirection dir);

    UIControl* FindFirstControl(UIControl* control, FocusHelpers::TabDirection dir);
    template <typename It>
    UIControl* FindFirstControlRecursive(It begin, It end, FocusHelpers::TabDirection dir);

    void PrepareChildren(UIControl* control, Vector<UIControl*>& children);

    RefPtr<UIControl> root;
};
}

#endif // __DAVAENGINE_TAB_TRAVERSAL_ALGORITHM_H__
