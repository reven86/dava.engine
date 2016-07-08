#ifndef __DAVAENGINE_UI_STYLESHEET_SYSTEM_H__
#define __DAVAENGINE_UI_STYLESHEET_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "UIStyleSheetStructs.h"

namespace DAVA
{
class UIControl;
class UIStyleSheet;
struct UIStyleSheetSelector;
class VariantType;

class UIStyleSheetSystem
{
public:
    UIStyleSheetSystem();
    ~UIStyleSheetSystem();

    void ProcessControl(UIControl* control);
    void AddGlobalClass(const FastName& clazz);
    void RemoveGlobalClass(const FastName& clazz);
    bool HasGlobalClass(const FastName& clazz) const;
    void SetGlobalTaggedClass(const FastName& tag, const FastName& clazz);
    void ResetGlobalTaggedClass(const FastName& tag);
    void ClearGlobalClasses();

private:
    bool StyleSheetMatchesControl(const UIStyleSheet* styleSheet, const UIControl* control);
    bool SelectorMatchesControl(const UIStyleSheetSelector& selector, const UIControl* control);

    template <typename CallbackType>
    void DoForAllPropertyInstances(UIControl* control, uint32 propertyIndex, const CallbackType& action);

    UIStyleSheetClassSet globalClasses;
};
};

#endif
