#include "Classes/Selection/Selection.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/DataProcessing/DataContext.h"

namespace Selection
{
SelectionData* GetSelectionData()
{
    DAVA::TArc::DataContext* activeContext = REGlobal::GetActiveContext();
    if (activeContext != nullptr)
    {
        return activeContext->GetData<SelectionData>();
    }
    return nullptr;
}

const SelectableGroup& GetSelection()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        return selectionData->GetSelection();
    }

    static SelectableGroup emptyGroup;
    return emptyGroup;
}

void CancelSelection()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        selectionData->CancelSelection();
    }
}

void SetSelection(SelectableGroup& newSelection)
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        selectionData->SetSelection(newSelection);
    }
}

DAVA::Entity* GetSelectableEntity(DAVA::Entity* selectionCandidate)
{
    DAVA::Entity* parent = selectionCandidate;
    while (nullptr != parent)
    {
        if (parent->GetSolid())
        {
            selectionCandidate = parent;
        }
        parent = parent->GetParent();
    }
    return selectionCandidate;
}

bool IsEntitySelectable(DAVA::Entity* selectionCandidate)
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        return selectionData->IsEntitySelectable(selectionCandidate);
    }

    return false;
}

void ResetSelectionComponentMask()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        selectionData->ResetSelectionComponentMask();
    }
}

void SetSelectionComponentMask(DAVA::uint64 mask)
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        selectionData->SetSelectionComponentMask(mask);
    }
}

DAVA::uint64 GetSelectionComponentMask()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        return selectionData->GetSelectionComponentMask();
    }

    return 0;
}

void SetSelectionAllowed(bool allowed)
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        selectionData->SetSelectionAllowed(allowed);
    }
}
bool IsSelectionAllowed()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        return selectionData->IsSelectionAllowed();
    }
    return false;
}

bool Lock()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        return selectionData->Lock(true);
    }
    return false;
}

void Unlock()
{
    SelectionData* selectionData = GetSelectionData();
    if (selectionData != nullptr)
    {
        selectionData->Lock(false);
    }
}
}
