#ifndef __DAVAENGINE_LOD_COMPONENT_H__
#define __DAVAENGINE_LOD_COMPONENT_H__

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

    enum eFlags
    {
        NEED_UPDATE_AFTER_LOAD = 1 << 0,
        RECURSIVE_UPDATE = 1 << 1
    };

    struct LodDistance : public InspBase
    {
        float32 distance;
        float32 nearDistanceSq;
        float32 farDistanceSq;

        LodDistance();
        void SetDistance(const float32& newDistance);
        float32 GetDistance() const
        {
            return distance;
        };

        void SetNearDistance(const float32& newDistance);
        void SetFarDistance(const float32& newDistance);

        float32 GetNearDistance() const;
        float32 GetFarDistance() const;

        INTROSPECTION(LodDistance,
                      PROPERTY("distance", "Distance", GetDistance, SetDistance, I_SAVE | I_VIEW)
                      MEMBER(nearDistanceSq, "Near Distance", I_SAVE | I_VIEW)
                      MEMBER(farDistanceSq, "Far Distance", I_SAVE | I_VIEW)
                      );
    };

    struct LodData
    {
        LodData()
            : layer(INVALID_LOD_LAYER)
            ,
            isDummy(false)
        {
        }

        Vector<Entity*> nodes;
        Vector<int32> indexes;
        int32 layer;
        bool isDummy;
    };

public:
    LodComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    static float32 GetDefaultDistance(int32 layer);

    DAVA_DEPRECATED(inline int32 GetLodLayersCount() const);
    inline float32 GetLodLayerDistance(int32 layerNum) const;
    inline float32 GetLodLayerNear(int32 layerNum) const;
    inline float32 GetLodLayerFar(int32 layerNum) const;
    inline float32 GetLodLayerNearSquare(int32 layerNum) const;
    inline float32 GetLodLayerFarSquare(int32 layerNum) const;

    DAVA_DEPRECATED(void GetLodData(Vector<LodData*>& retLodLayers));

    int32 currentLod;
    Vector<LodData> lodLayers;
    Vector<LodDistance> lodLayersArray;
    int32 forceLodLayer;

    void SetForceDistance(const float32& newDistance);
    float32 GetForceDistance() const;
    float32 forceDistance;
    float32 forceDistanceSq;

    int32 flags;

    /**
         \brief Registers LOD layer into the LodComponent.
         \param[in] layerNum is the layer index
         \param[in] distance near view distance for the layer
	 */
    void SetLodLayerDistance(int32 layerNum, float32 distance);

    /**
         \brief Sets lod layer that would be forcely used in the whole scene.
         \param[in] layer layer to set on the for the scene. Use -1 to disable forced lod layer.
	 */
    void SetForceLodLayer(int32 layer);
    int32 GetForceLodLayer() const;

    int32 GetMaxLodLayer() const;

    void CopyLODSettings(const LodComponent* fromLOD);

    inline void EnableRecursiveUpdate();
    inline bool IsRecursiveUpdate();

public:
    INTROSPECTION_EXTEND(LodComponent, Component,
                         COLLECTION(lodLayersArray, "Lod Layers Array", I_SAVE | I_VIEW)
                         MEMBER(forceLodLayer, "Force Lod Layer", I_SAVE | I_VIEW)
                         PROPERTY("forceDistance", "Force Distance", GetForceDistance, SetForceDistance, I_SAVE | I_VIEW)
                         MEMBER(flags, "Flags", I_SAVE | I_VIEW | I_EDIT)
                         );
};

int32 LodComponent::GetLodLayersCount() const
{
    return static_cast<int32>(lodLayers.size());
}

float32 LodComponent::GetLodLayerDistance(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].distance;
}

float32 LodComponent::GetLodLayerNearSquare(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].nearDistanceSq;
}

float32 LodComponent::GetLodLayerFarSquare(int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].farDistanceSq;
}

void LodComponent::EnableRecursiveUpdate()
{
    flags |= RECURSIVE_UPDATE;
}

bool LodComponent::IsRecursiveUpdate()
{
    return (flags & RECURSIVE_UPDATE) != 0;
}
};

#endif //__DAVAENGINE_LOD_COMPONENT_H__
