#ifndef __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
#define __DAVAENGINE_UI_LAYOUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "UI/Layouts/ControlLayoutData.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIControl;

class UILayoutSystem
: public UISystem
{
public:
    UILayoutSystem();
    ~UILayoutSystem() override;

    void Process(DAVA::float32 elapsedTime) override{};

    bool IsRtl() const;
    void SetRtl(bool rtl);

    bool IsAutoupdatesEnabled() const;
    void SetAutoupdatesEnabled(bool enabled);

    void ProcessControl(UIControl* control);
    void ManualApplyLayout(UIControl* control);

private:
    void ApplyLayout(UIControl* control);
    void ApplyLayoutNonRecursive(UIControl* control);

    UIControl* FindNotDependentOnChildrenControl(UIControl* control) const;
    bool HaveToLayoutAfterReorder(const UIControl* control) const;
    bool HaveToLayoutAfterReposition(const UIControl* control) const;

    void CollectControls(UIControl* control, bool recursive);
    void CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive);

    void ProcessAxis(Vector2::eAxis axis);
    void DoMeasurePhase(Vector2::eAxis axis);
    void DoLayoutPhase(Vector2::eAxis axis);

    void ApplySizesAndPositions();
    void ApplyPositions();

    bool isRtl = false;
    bool autoupdatesEnabled = true;
    Vector<ControlLayoutData> layoutData;
};
}


#endif //__DAVAENGINE_UI_LAYOUT_SYSTEM_H__
