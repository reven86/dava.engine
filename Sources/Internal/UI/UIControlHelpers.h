#ifndef __DAVAENGINE_UI_CONTROL_HELPERS_H__
#define __DAVAENGINE_UI_CONTROL_HELPERS_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Math/Rect.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIScrollView;

class UIControlHelpers
{
public:
    static String GetControlPath(const UIControl* control, const UIControl* rootControl = NULL);
    static String GetPathToOtherControl(const UIControl* src, const UIControl* dst);
    static UIControl* FindChildControlByName(const String& controlName, const UIControl* rootControl, bool recursive);
    static UIControl* FindChildControlByName(const FastName& controlName, const UIControl* rootControl, bool recursive);
    static UIControl* FindControlByPath(const String& controlPath, UIControl* rootControl);
    static const UIControl* FindControlByPath(const String& controlPath, const UIControl* rootControl);

    static void ScrollToControl(DAVA::UIControl* control, bool toTopLeftForBigControls = false);
    static void ScrollToControlWithAnimation(DAVA::UIControl* control, float32 animationTime = 0.3f, bool toTopLeftForBigControls = false);

    static bool IsControlNameValid(const FastName& controlName);

private:
    static const UIControl* FindControlByPathImpl(const String& controlPath, const UIControl* rootControl);
    static const UIControl* FindControlByPathImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, const UIControl* rootControl);
    static const UIControl* FindControlByPathRecursivelyImpl(Vector<FastName>::const_iterator begin, Vector<FastName>::const_iterator end, const UIControl* rootControl);

    static void ScrollToRect(DAVA::UIControl* control, const Rect& rect, float32 animationTime, bool toTopLeftForBigControls);
    static float32 GetScrollPositionToShowControl(float32 controlPos, float32 controlSize, float32 scrollSize, bool toTopLeftForBigControls);
    static Rect ScrollListToRect(UIList* list, const DAVA::Rect& rect, float32 animationTime, bool toTopLeftForBigControls);
    static Rect ScrollUIScrollViewToRect(UIScrollView* scrollView, const DAVA::Rect& rect, float32 animationTime, bool toTopLeftForBigControls);
};
};
#endif // __DAVAENGINE_UI_CONTROL_HELPERS_H__
