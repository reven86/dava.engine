#ifndef __DAVAENGINE_SCENE3D_LODSYSTEM_H__
#define __DAVAENGINE_SCENE3D_LODSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class Camera;
class LodComponent;

class LodSystem : public SceneSystem
{
public:
    LodSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    virtual void SetCamera(Camera* camera);
    inline Camera* GetCamera() const;

    inline void SetForceUpdateAll();

    static void UpdateEntitiesAfterLoad(Entity* entity);
    static void UpdateEntityAfterLoad(Entity* entity);

    static void MergeChildLods(Entity* toEntity);

    static void ForceUpdate(Entity* entity, Camera* camera, float32 timeElapsed);

    class LodMerger
    {
    public:
        LodMerger(Entity* toEntity);
        void MergeChildLods();

    private:
        void GetLodComponentsRecursive(Entity* fromEntity, Vector<Entity*>& allLods);
        Entity* toEntity;
    };

private:
    //partial update per frame
    static const int32 UPDATE_PART_PER_FRAME = 10;
    Vector<int32> partialUpdateIndices;
    int32 currentPartialUpdateIndex;
    void UpdatePartialUpdateIndices();
    bool forceUpdateAll;

    Vector<Entity*> entities;

    static void UpdateLod(Entity* entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);
    static bool RecheckLod(Entity* entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);

    static float32 CalculateDistanceToCamera(const Entity* entity, const LodComponent* lodComponent, Camera* camera);
    static int32 FindProperLayer(float32 distance, const LodComponent* lodComponent, int32 requestedLayersCount);

    static inline void ProcessEntity(Entity* entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);
    static inline void PorcessEntityRecursive(Entity* entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);

    static void SetEntityLodRecursive(Entity* entity, int32 currentLod);
    static void SetEntityLod(Entity* entity, int32 currentLod);

    Camera* camera;
};

void LodSystem::ProcessEntity(Entity* entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera)
{
    LodComponent* lod = GetLodComponent(entity);
    if (lod->flags & LodComponent::NEED_UPDATE_AFTER_LOAD)
    {
        UpdateEntityAfterLoad(entity);
    }

    UpdateLod(entity, lod, psLodOffsetSq, psLodMultSq, camera);
}

void LodSystem::SetForceUpdateAll()
{
    forceUpdateAll = true;
}

Camera* LodSystem::GetCamera() const
{
    return camera;
}

inline void LodSystem::SetEntityLod(Entity* entity, int32 currentLod)
{
    RenderObject* ro = GetRenderObject(entity);
    if (ro)
    {
        if (currentLod == LodComponent::LAST_LOD_LAYER)
        {
            ro->SetLodIndex(ro->GetMaxLodIndex());
        }
        else
        {
            ro->SetLodIndex(currentLod);
        }
    }
}
}

#endif //__DAVAENGINE_SCENE3D_LODSYSTEM_H__