#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "UI/UISystem.h"

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
    void UpdateControl(UIControl* control);

    UIControl* FindNotDependentOnChildrenControl(UIControl* control) const;
    bool HaveToLayoutAfterReorder(const UIControl* control) const;
    bool HaveToLayoutAfterReposition(const UIControl* control) const;

    bool isRtl = false;
    bool autoupdatesEnabled = true;
    bool dirty = false;
    bool needUpdate = false;
    std::unique_ptr<class Layouter> sharedLayouter;
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
