#ifndef __DAVAENGINE_SWITCHTORENDEROBJECTCONVERTER_H__
#define __DAVAENGINE_SWITCHTORENDEROBJECTCONVERTER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
class PolygonGroup;
class RenderObject;

class SwitchToRenerObjectConverter
{
public:
    void ConsumeSwitchedRenderObjects(Entity* scene);
    void SerachForSwitch(Entity* currentNode);
    bool MergeSwitch(Entity* entity);

private:
    void FindRenderObjectsRecursive(Entity* fromEntity, Vector<std::pair<Entity*, RenderObject*>>& entityAndObjectPairs);
    Set<PolygonGroup*> bakedPolygonGroups;
};
};

#endif //__DAVAENGINE_SWITCHTORENDEROBJECTCONVERTER_H__