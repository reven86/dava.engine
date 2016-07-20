#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
class Entity;
class LodComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(LOD_COMPONENT);

    static const int32 MAX_LOD_LAYERS = 4;
    static const int32 INVALID_LOD_LAYER = -1;
    static const float32 MIN_LOD_DISTANCE;
    static const float32 MAX_LOD_DISTANCE;
    static const float32 INVALID_DISTANCE;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetLodLayerDistance(int32 layerNum, float32 distance);
    float32 GetLodLayerDistance(int32 layerNum) const;

    int32 GetCurrentLod() const;

    void EnableRecursiveUpdate();

private:
    int32 currentLod = INVALID_LOD_LAYER;
    Array<float32, MAX_LOD_LAYERS> distances = Array<float32, MAX_LOD_LAYERS>{ 300.f, 600.f, 900.f, 1000.f }; //cause list initialization for members not implemented in MSVC https://msdn.microsoft.com/en-us/library/dn793970.aspx

    friend class LodSystem;

public:
    INTROSPECTION_EXTEND(LodComponent, Component,
                         MEMBER(currentLod, "Current Lod", I_VIEW)
                         )
};

REGISTER_CLASS(LodComponent);

inline void LodComponent::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    distances[layerNum] = distance;
    GlobalEventSystem::Instance()->Event(this, EventSystem::LOD_DISTANCE_CHANGED);
}

inline void LodComponent::EnableRecursiveUpdate()
{
    GlobalEventSystem::Instance()->Event(this, EventSystem::LOD_RECURSIVE_UPDATE_ENABLED);
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
