#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA
{
class Entity;
class SpeedTreeObject;
class PolygonGroup;
class RenderBatch;
class NMaterial;

class SpeedTreeConverter
{
public:
    static void CalculateAnimationParams(SpeedTreeObject* object);
    static float32 CalculateVertexFlexibility(PolygonGroup* pg, int32 vi, float32 treeHeight);
    static Vector2 CalculateVertexAngle(PolygonGroup* pg, int32 vi, float32 treeHeight);

    void ConvertTrees(Entity* scene);
    void ConvertPolygonSortedGroups(Entity* scene);

private:
    void ConvertingPathRecursive(Entity* scene);
    void ConvertLeafPGForAnimations(PolygonGroup* geometry);
    void ConvertTrunkForAnimations(PolygonGroup* geometry);

    bool IsTreeLeafBatch(RenderBatch* batch); //legacy

    Set<PolygonGroup*> uniqPGs;
    Set<PolygonGroup*> uniqLeafPGs;
    Set<PolygonGroup*> uniqTrunkPGs;
    Set<NMaterial*> materials;
    Set<SpeedTreeObject*> uniqTreeObjects;
};
};
