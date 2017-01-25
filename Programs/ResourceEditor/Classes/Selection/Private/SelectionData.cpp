#include "Classes/Selection/SelectionData.h"
#include "Classes/Selection/SelectionSystem.h"

const char* SelectionData::selectionPropertyName = "selection";
const char* SelectionData::selectionBoxPropertyName = "selectionBox";
const char* SelectionData::selectionAllowedPropertyName = "selectionAllowed";

const SelectableGroup& SelectionData::GetSelection() const
{
    DVASSERT(selectionSystem);
    return selectionSystem->GetSelection();
}

void SelectionData::SetSelection(SelectableGroup& newSelection)
{
    DVASSERT(selectionSystem);
    selectionSystem->SetSelection(newSelection);
}

void SelectionData::CancelSelection()
{
    DVASSERT(selectionSystem);
    selectionSystem->CancelSelection();
}

void SelectionData::ResetSelectionComponentMask()
{
    DVASSERT(selectionSystem);
    selectionSystem->ResetSelectionComponentMask();
}

void SelectionData::SetSelectionComponentMask(DAVA::uint64 mask)
{
    DVASSERT(selectionSystem);
    selectionSystem->SetSelectionComponentMask(mask);
}

DAVA::uint64 SelectionData::GetSelectionComponentMask() const
{
    DVASSERT(selectionSystem);
    return selectionSystem->GetSelectionComponentMask();
}

void SelectionData::SetSelectionAllowed(bool allowed)
{
    DVASSERT(selectionSystem);
    selectionSystem->SetSelectionAllowed(allowed);
}

bool SelectionData::IsSelectionAllowed() const
{
    DVASSERT(selectionSystem);
    return selectionSystem->IsSelectionAllowed();
}

bool SelectionData::Lock()
{
    DVASSERT(selectionSystem);
    bool wasLocked = selectionSystem->IsLocked();
    selectionSystem->SetLocked(true);
    return wasLocked;
}

void SelectionData::Unlock()
{
    DVASSERT(selectionSystem);
    selectionSystem->SetLocked(false);
}

bool SelectionData::IsEntitySelectable(DAVA::Entity* selectionCandidate) const
{
    DVASSERT(selectionSystem);
    return selectionSystem->IsEntitySelectable(selectionCandidate);
}

const DAVA::AABBox3& SelectionData::GetSelectionBox() const
{
    DVASSERT(selectionSystem);
    return selectionSystem->GetSelectionBox();
}
