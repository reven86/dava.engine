#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class Entity;
class LodComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(LOD_COMPONENT);

    static const int32 MAX_LOD_LAYERS = 4;
    static const int32 INVALID_LOD_LAYER = -1;
    static const int32 LAST_LOD_LAYER = 0x7fffffff;
    static const float32 MIN_LOD_DISTANCE;
    static const float32 MAX_LOD_DISTANCE;
    static const float32 INVALID_DISTANCE;

    LodComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetLodLayerDistance(int32 layerNum, float32 distance);
    float32 GetLodLayerDistance(int32 layerNum) const;

    int32 GetCurrentLod() const;

    void EnableRecursiveUpdate();

private:
    static float32 GetDefaultDistance(int32 layer);
    int32 currentLod;
    Vector<float32> distances;
    void SetCurrentLod(int32 lod);

public:
    INTROSPECTION_EXTEND(LodComponent, Component,
                         COLLECTION(distances, "Lod Distances", I_SAVE | I_VIEW)
                         );

    friend class LodSystem;
};

REGISTER_CLASS(LodComponent);

inline void LodComponent::SetCurrentLod(int32 lod)
{
    currentLod = lod;
}

inline int32 LodComponent::GetCurrentLod() const
{
    return currentLod;
}

inline float32 LodComponent::GetLodLayerDistance(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return distances[layerNum];
}

};
