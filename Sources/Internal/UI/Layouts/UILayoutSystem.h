#ifndef __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
#define __DAVAENGINE_UI_LAYOUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
class UIControl;

class UILayoutSystem
{
public:
    UILayoutSystem();
    virtual ~UILayoutSystem();

public:
    bool IsRtl() const;
    void SetRtl(bool rtl);

    bool IsAutoupdatesEnabled() const;
    void SetAutoupdatesEnabled(bool enabled);

    void ApplyLayout(UIControl* control, bool considerDenendenceOnChildren = false);
    void ApplyLayoutNonRecursive(UIControl* control);

private:
    UIControl* FindNotDependentOnChildrenControl(UIControl* control) const;

    void CollectControls(UIControl* control, bool recursive);
    void CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive);

    void ProcessAxis(Vector2::eAxis axis);
    void DoMeasurePhase(Vector2::eAxis axis);
    void DoLayoutPhase(Vector2::eAxis axis);

    void ApplySizesAndPositions();
    void ApplyPositions();

private:
    bool isRtl = false;
    bool autoupdatesEnabled = true;
    Vector<ControlLayoutData> layoutData;
};
}


#endif //__DAVAENGINE_UI_LAYOUT_SYSTEM_H__
