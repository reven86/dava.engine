#ifndef __DAVAENGINE_ANIMATEDTREE_CONVERTER_H__
#define __DAVAENGINE_ANIMATEDTREE_CONVERTER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
class SpeedTreeObject;
class PolygonGroup;

class TreeToAnimatedTreeConverter
{
public:
    static void CalculateAnimationParams(SpeedTreeObject* object);

    void ConvertTrees(Entity* scene);

private:
    void ConvertingPathRecursive(Entity* scene);
    void ConvertLeafPGForAnimations(PolygonGroup* geometry);
    void ConvertTrunkForAnimations(PolygonGroup* geometry);

    Set<PolygonGroup*> uniqLeafPGs;
    Set<PolygonGroup*> uniqTrunkPGs;
    Set<SpeedTreeObject*> uniqTreeObjects;
};
};

#endif //__DAVAENGINE_ANIMATEDTREE_CONVERTER_H__
