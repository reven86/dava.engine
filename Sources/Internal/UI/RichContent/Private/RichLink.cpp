#include "UI/RichContent/Private/RichLink.h"

#include "UI/RichContent/UIRichContentAliasesComponent.h"
#include "UI/RichContent/UIRichContentComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
void RichLink::AddItem(const RefPtr<UIControl>& item)
{
    richItems.push_back(item);
}

void RichLink::RemoveItems()
{
    if (component != nullptr)
    {
        UIControl* ctrl = component->GetControl();
        if (ctrl != nullptr)
        {
            for (const RefPtr<UIControl>& item : richItems)
            {
                ctrl->RemoveControl(item.Get());
            }
        }
    }
    richItems.clear();
}

void RichLink::AddAliases(UIRichContentAliasesComponent* component)
{
    auto it = std::find(aliasesComponents.begin(), aliasesComponents.end(), component);
    if (it == aliasesComponents.end())
    {
        aliasesComponents.push_back(component);
    }
}

void RichLink::RemoveAliases(UIRichContentAliasesComponent* component)
{
    auto it = std::find(aliasesComponents.begin(), aliasesComponents.end(), component);
    if (it != aliasesComponents.end())
    {
        aliasesComponents.erase(it);
    }
}
}