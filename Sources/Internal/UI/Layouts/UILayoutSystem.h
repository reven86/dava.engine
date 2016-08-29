#pragma once

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

    void Update(UIControl* root);
    void SetDirty();
    void CheckDirty();

private:
    UIControl* FindNotDependentOnChildrenControl(UIControl* control) const;

    void CollectControls(UIControl* control, bool recursive);
    void CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive);

    void ProcessAxis(Vector2::eAxis axis);
    void DoMeasurePhase(Vector2::eAxis axis);
    void DoLayoutPhase(Vector2::eAxis axis);

    void ApplySizesAndPositions();
    void ApplyPositions();

    void UpdateControl(UIControl* control);

private:
    bool isRtl = false;
    bool autoupdatesEnabled = true;
    bool dirty = false;
    bool needUpdate = false;
    Vector<ControlLayoutData> layoutData;
};

inline void UILayoutSystem::SetDirty()
{
    dirty = true;
}

inline void UILayoutSystem::CheckDirty()
{
    needUpdate = dirty;
    dirty = false;
}
}
