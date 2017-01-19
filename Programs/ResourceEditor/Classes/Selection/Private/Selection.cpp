#include "Classes/Selection/Selection.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/DataProcessing/DataContext.h"

namespace Selection
{
const SelectableGroup& GetSelection()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->GetSelection();
    }

    static SelectableGroup emptyGroup;
    return emptyGroup;
}

void CancelSelection()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->CancelSelection();
    }
}

void SetSelection(SelectableGroup& newSelection)
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
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
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->IsEntitySelectable(selectionCandidate);
    }

    return false;
}

void ResetSelectionComponentMask()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->ResetSelectionComponentMask();
    }
}

void SetSelectionComponentMask(DAVA::uint64 mask)
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->SetSelectionComponentMask(mask);
    }
}

DAVA::uint64 GetSelectionComponentMask()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->GetSelectionComponentMask();
    }

    return 0;
}

bool Lock()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->Lock();
    }
    return false;
}

void Unlock()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->Unlock();
    }
}
}
