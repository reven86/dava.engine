#pragma once

#include "Base/Vector.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class UIControl;
class UIRichContentAliasesComponent;
class UIRichContentComponent;

struct RichLink final
{
    UIControl* control = nullptr;
    UIRichContentComponent* component = nullptr;
    Vector<UIRichContentAliasesComponent*> aliasesComponents;
    Vector<RefPtr<UIControl>> richItems;

    void AddItem(const RefPtr<UIControl>& item);
    void RemoveItems();
    void AddAliases(UIRichContentAliasesComponent* component);
    void RemoveAliases(UIRichContentAliasesComponent* component);
};
}