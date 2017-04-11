#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
class SpeedTreeObject;
class PolygonGroup;

class SpeedTreeConverter
{
public:
    static void CalculateAnimationParams(SpeedTreeObject* object);

    void ConvertTrees(Entity* scene);
    void ConvertPolygonPivotGroups(Entity* scene);

private:
    void ConvertingPathRecursive(Entity* scene);
    void ConvertLeafPGForAnimations(PolygonGroup* geometry);
    void ConvertTrunkForAnimations(PolygonGroup* geometry);

    Set<PolygonGroup*> uniqPGs;
    Set<PolygonGroup*> uniqLeafPGs;
    Set<PolygonGroup*> uniqTrunkPGs;
    Set<NMaterial*> materials;
    Set<SpeedTreeObject*> uniqTreeObjects;
};
};
