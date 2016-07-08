#ifndef __SYSTEM_DELEGATES_H__
#define __SYSTEM_DELEGATES_H__

namespace DAVA
{
class Entity;
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
    virtual bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection) = 0;
    virtual bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection) = 0;
};

#endif //__SYSTEM_DELEGATES_H__
