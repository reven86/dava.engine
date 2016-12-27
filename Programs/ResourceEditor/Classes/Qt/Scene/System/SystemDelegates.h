#pragma once

namespace DAVA
{
class Entity;
class AABBox3;
}

class SelectableGroup;

class EntityModificationSystemDelegate
{
public:
    virtual void WillClone(DAVA::Entity* originalEntity) = 0;
    virtual void DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity) = 0;
};

class StructureSystemDelegate
{
public:
    virtual void WillRemove(DAVA::Entity* removedEntity) = 0;
    virtual void DidRemoved(DAVA::Entity* removedEntity) = 0;
};

class SceneSelectionSystemDelegate
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
    virtual void OnSelectionBoxChanged(const DAVA::AABBox3& newBox)
    {
    }
};
