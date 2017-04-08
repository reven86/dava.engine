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

struct UIStyleSheetProcessDebugData
{
    UIStyleSheetProcessDebugData()
    {
        std::fill(propertySources.begin(), propertySources.end(), nullptr);
    }

    DAVA::Vector<UIPriorityStyleSheet> styleSheets;
    DAVA::Array<const UIStyleSheet*, UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT> propertySources;
    UIStyleSheetPropertySet appliedProperties;
};

class UIStyleSheetSystem
: public UISystem
{
public:
    UIStyleSheetSystem();
    ~UIStyleSheetSystem() override;

    void Process(DAVA::float32 elapsedTime) override;

    void SetCurrentScreen(const RefPtr<UIScreen>& screen);
    void SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition);
    void SetPopupContainer(const RefPtr<UIControl>& popupContainer);

    void ProcessControl(UIControl* control, bool styleSheetListChanged = false);
    void DebugControl(UIControl* control, UIStyleSheetProcessDebugData* debugData);

    void AddGlobalClass(const FastName& clazz);
    void RemoveGlobalClass(const FastName& clazz);
    bool HasGlobalClass(const FastName& clazz) const;
    void SetGlobalTaggedClass(const FastName& tag, const FastName& clazz);
    FastName GetGlobalTaggedClass(const FastName& tag) const;
    void ResetGlobalTaggedClass(const FastName& tag);
    void ClearGlobalClasses();

    void ClearStats();
    void DumpStats();

    void Update(UIControl* root);
    void SetDirty();
    void CheckDirty();

private:
    void ProcessControl(UIControl* control, int32 distanceFromDirty, bool styleSheetListChanged, bool recursively, bool dryRun, UIStyleSheetProcessDebugData* debugData);
    void UpdateControl(UIControl* root);

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
