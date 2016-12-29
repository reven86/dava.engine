#pragma once

namespace DAVA
{
class Entity;
class AABBox3;
}

class EntityModificationSystemDelegate
{
public:
    virtual void WillClone(DAVA::Entity* originalEntity) = 0;
    virtual void DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity) = 0;
};

class SelectableGroup;
class StructureSystemDelegate
{
public:
    virtual void WillRemove(DAVA::Entity* removedEntity) = 0;
    virtual void DidRemoved(DAVA::Entity* removedEntity) = 0;
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
