#ifndef __DAVAENGINE_UI_STYLESHEET_SYSTEM_H__
#define __DAVAENGINE_UI_STYLESHEET_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "UI/Styles/UIPriorityStyleSheet.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include "UI/Styles/UIStyleSheetStructs.h"
#include "UI/UISystem.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class UIControl;
class UIScreen;
class UIScreenTransition;
class UIStyleSheet;
struct UIStyleSheetSelector;
class VariantType;

class UIStyleSheetSystemListener
{
public:
    UIStyleSheetSystemListener()
    {
    }
    virtual ~UIStyleSheetSystemListener()
    {
    }

    virtual void OnStylePropertyChanged(UIControl* control, UIComponent* component, uint32 propertyIndex) = 0;
};

struct UIStyleSheetProcessDebugData
{
    UIStyleSheetProcessDebugData()
    {
        std::fill(propertySources.begin(), propertySources.end(), nullptr);
    }

    Vector<UIPriorityStyleSheet> styleSheets;
    Array<const UIStyleSheet*, UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT> propertySources;
    UIStyleSheetPropertySet appliedProperties;
};

class UIStyleSheetSystem
: public UISystem
{
public:
    UIStyleSheetSystem();
    ~UIStyleSheetSystem() override;

    void SetCurrentScreen(const RefPtr<UIScreen>& screen);
    void SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition);
    void SetPopupContainer(const RefPtr<UIControl>& popupContainer);
    void SetListener(UIStyleSheetSystemListener* listener);

    void AddGlobalClass(const FastName& clazz);
    void RemoveGlobalClass(const FastName& clazz);
    bool HasGlobalClass(const FastName& clazz) const;
    void SetGlobalTaggedClass(const FastName& tag, const FastName& clazz);
    FastName GetGlobalTaggedClass(const FastName& tag) const;
    void ResetGlobalTaggedClass(const FastName& tag);
    void ClearGlobalClasses();

    void ClearStats();
    void DumpStats();

    void SetDirty();
    void CheckDirty();

    void DebugControl(UIControl* control, UIStyleSheetProcessDebugData* debugData);
    void ProcessControl(UIControl* control, bool styleSheetListChanged = false); //DON'T USE IT!

private:
    void Process(float32 elapsedTime) override;
    void ForceProcessControl(float32 elapsedTime, UIControl* control) override;

    void ProcessControlImpl(UIControl* control, int32 distanceFromDirty, bool styleSheetListChanged, bool recursively, bool dryRun, UIStyleSheetProcessDebugData* debugData);
    void ProcessControlHierarhy(UIControl* root);

    bool StyleSheetMatchesControl(const UIStyleSheet* styleSheet, const UIControl* control);
    bool SelectorMatchesControl(const UIStyleSheetSelector& selector, const UIControl* control);

    template <typename CallbackType>
    void DoForAllPropertyInstances(UIControl* control, uint32 propertyIndex, const CallbackType& action);

    UIStyleSheetClassSet globalClasses;

    uint64 statsTime = 0;
    int32 statsProcessedControls = 0;
    int32 statsMatches = 0;
    int32 statsStyleSheetCount = 0;
    bool dirty = false;
    bool needUpdate = false;
    RefPtr<UIScreen> currentScreen;
    RefPtr<UIControl> popupContainer;
    RefPtr<UIScreenTransition> currentScreenTransition;

    UIStyleSheetSystemListener* listener = nullptr;
};

inline void UIStyleSheetSystem::SetDirty()
{
    dirty = true;
}

inline void UIStyleSheetSystem::CheckDirty()
{
    needUpdate = dirty;
    dirty = false;
}
};

#endif
