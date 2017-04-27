#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "UI/Layouts/ControlLayoutData.h"
#include "UI/UISystem.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class UIControl;
class UIScreen;
class UIScreenTransition;

class UILayoutSystemListener
{
public:
    virtual ~UILayoutSystemListener() = default;

    virtual void OnControlLayouted(UIControl* control) = 0;
    virtual void OnFormulaChanged(UIControl* control, Vector2::eAxis axis) = 0;
};

class UILayoutSystem : public UISystem
{
public:
    UILayoutSystem();
    ~UILayoutSystem() override;

    void Process(DAVA::float32 elapsedTime) override;

    void SetCurrentScreen(const RefPtr<UIScreen>& screen);
    void SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition);
    void SetPopupContainer(const RefPtr<UIControl>& popupContainer);

    bool IsRtl() const;
    void SetRtl(bool rtl);

    bool IsAutoupdatesEnabled() const;
    void SetAutoupdatesEnabled(bool enabled);

    void ProcessControl(UIControl* control);
    void ManualApplyLayout(UIControl* control);

    void Update(UIControl* root);
    void SetDirty();
    void CheckDirty();

    UILayoutSystemListener* GetListener() const;
    void SetListener(UILayoutSystemListener* listener);

private:
    void ApplyLayout(UIControl* control);
    void ApplyLayoutNonRecursive(UIControl* control);

    UIControl* FindNotDependentOnChildrenControl(UIControl* control) const;
    bool HaveToLayoutAfterReorder(const UIControl* control) const;
    bool HaveToLayoutAfterReposition(const UIControl* control) const;

    void CollectControls(UIControl* control, bool recursive);
    void CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive);

    void ProcessAxis(Vector2::eAxis axis, bool processSizes);
    void DoMeasurePhase(Vector2::eAxis axis);
    void DoLayoutPhase(Vector2::eAxis axis);

    void ApplySizesAndPositions();
    void ApplyPositions();

    void UpdateControl(UIControl* control);

    bool isRtl = false;
    bool autoupdatesEnabled = true;
    bool dirty = false;
    bool needUpdate = false;
    Vector<ControlLayoutData> layoutData;
    RefPtr<UIScreen> currentScreen;
    RefPtr<UIControl> popupContainer;
    RefPtr<UIScreenTransition> currentScreenTransition;

    UILayoutSystemListener* listener = nullptr;
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
