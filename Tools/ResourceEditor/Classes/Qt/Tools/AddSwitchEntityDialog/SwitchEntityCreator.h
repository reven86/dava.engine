#ifndef __SWITCH_ENTITY_CREATEOR_H__
#define __SWITCH_ENTITY_CREATEOR_H__

#include "DAVAEngine.h"

typedef std::pair<DAVA::Entity*, DAVA::RenderObject*> RENDER_PAIR;

class SwitchEntityCreator
{
    static const DAVA::uint32 MAX_SWITCH_COUNT = 3;

public:
    DAVA::Entity* CreateSwitchEntity(const DAVA::Vector<DAVA::Entity*>& fromEntities);
    bool HasSwitchComponentsRecursive(DAVA::Entity* fromEntity);
    bool HasRenderObjectsRecursive(DAVA::Entity* fromEntity);

protected:
    void CreateSingleObjectData(DAVA::Entity* switchEntity);
    void CreateMultipleObjectsData();

    void FindRenderObjectsRecursive(DAVA::Entity* fromEntity, DAVA::Vector<RENDER_PAIR>& entityAndObjectPairs);

    DAVA::Vector<DAVA::Entity*> clonedEntities;
    DAVA::Vector<DAVA::Entity*> realChildren;

    DAVA::Vector<RENDER_PAIR> renderPairs[MAX_SWITCH_COUNT];
};

#endif // __SWITCH_ENTITY_CREATEOR_H__
