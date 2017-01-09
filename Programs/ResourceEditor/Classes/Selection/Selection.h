#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
}

class SelectableGroup;

namespace Selection
{
const SelectableGroup& GetSelection();
void SetSelection(SelectableGroup& newSelection);

void CancelSelection();

DAVA::Entity* GetSelectableEntity(DAVA::Entity* selectionCandidate);
bool IsEntitySelectable(DAVA::Entity* selectionCandidate);

void ResetSelectionComponentMask();
void SetSelectionComponentMask(DAVA::uint64 mask);
DAVA::uint64 GetSelectionComponentMask();

bool Lock();
void Unlock();
}
