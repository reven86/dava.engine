#ifndef __DAVAENGINE_LODTOLOD2CONVERTER_H__
#define __DAVAENGINE_LODTOLOD2CONVERTER_H__

#include "Base/BaseTypes.h"
namespace DAVA
{
class Entity;
class PolygonGroup;
class RenderObject;
class LodToLod2Converter
{
public:
    void ConvertLodToV2(Entity* scene);
    void SearchForLod(Entity* currentNode);
    bool MergeLod(Entity* entity);

private:
    void FindAndEraseRenderObjectsRecursive(Entity* fromEntity, Vector<std::pair<Entity*, RenderObject*>>& entitiesAndRenderObjects);
    Set<PolygonGroup*> bakedPolygonGroups;
};
};

#endif //__DAVAENGINE_LODTOLOD2CONVERTER_H__
