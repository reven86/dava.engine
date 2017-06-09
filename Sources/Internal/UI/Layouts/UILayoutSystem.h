#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "UI/UISystem.h"

struct UILayoutSystemTest;

namespace DAVA
{
class UIControl;
class UIScreen;
class UIScreenTransition;
class UILayoutSystemListener;

class UILayoutSystem : public UISystem
{
public:
    UILayoutSystem();
    ~UILayoutSystem() override;

    void SetCurrentScreen(const RefPtr<UIScreen>& screen);
    void SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition);
    void SetPopupContainer(const RefPtr<UIControl>& popupContainer);

    bool IsRtl() const;
    void SetRtl(bool rtl);

    bool IsAutoupdatesEnabled() const;
    void SetAutoupdatesEnabled(bool enabled);

    void SetDirty();
    void CheckDirty();

    void AddListener(UILayoutSystemListener* listener);
    void RemoveListener(UILayoutSystemListener* listener);

    void ManualApplyLayout(UIControl* control); //DON'T USE IT!

protected:
    void Process(float32 elapsedTime) override;
    void ForceProcessControl(float32 elapsedTime, UIControl* control) override;

    void UnregisterControl(UIControl* control) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

private:
    UIControl* FindNotDependentOnChildrenControl(UIControl* control) const;
    bool HaveToLayoutAfterReorder(const UIControl* control) const;
    bool HaveToLayoutAfterReposition(const UIControl* control) const;

    void CollectControls(UIControl* control, bool recursive);
    void CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive);
    void ProcessControlHierarhy(UIControl* control);
    void ProcessControl(UIControl* control);

    bool isRtl = false;
    bool autoupdatesEnabled = true;
    bool dirty = false;
    bool needUpdate = false;
    std::unique_ptr<class Layouter> sharedLayouter;
    RefPtr<UIScreen> currentScreen;
    RefPtr<UIControl> popupContainer;
    RefPtr<UIScreenTransition> currentScreenTransition;

    Vector<UILayoutSystemListener*> listeners;

    friend UILayoutSystemTest;
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
