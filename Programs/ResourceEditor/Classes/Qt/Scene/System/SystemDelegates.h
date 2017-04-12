#pragma once

#include <Debug/DVAssert.h>

namespace DAVA
{
class Entity;
class AABBox3;
}

class EntityModificationSystemDelegate
{
public:
    virtual bool HasCustomClonedAddading(DAVA::Entity* entityToClone) const
    {
        return false;
    }

    virtual void PerformAdding(DAVA::Entity* sourceEntity, DAVA::Entity* clonedEntity)
    {
        DVASSERT(false);
    }
    virtual void WillClone(DAVA::Entity* originalEntity)
    {
    }
    virtual void DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity)
    {
    }
};

class SelectableGroup;
class StructureSystemDelegate
{
public:
    virtual bool HasCustomRemovingForEntity(DAVA::Entity* entityToRemove) const
    {
        return false;
    }

    virtual void PerformRemoving(DAVA::Entity* entityToRemove)
    {
        DVASSERT(false);
    }

    virtual void WillRemove(DAVA::Entity* removedEntity)
    {
    }
    virtual void DidRemoved(DAVA::Entity* removedEntity)
    {
    }
};

class SelectionSystemDelegate
{
public:
    virtual bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
    {
        return true;
    }
    virtual bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
    {
        return true;
    }
};
