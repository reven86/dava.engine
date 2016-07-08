#ifndef __VISIBILITYCHECKSYSTEM_H__
#define __VISIBILITYCHECKSYSTEM_H__

#include "VisibilityCheckRenderer.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Landscape;
}

class Command2;
class VisibilityCheckSystem : public DAVA::SceneSystem, VisibilityCheckRendererDelegate
{
public:
    static void ReleaseCubemapRenderTargets();

public:
    VisibilityCheckSystem(DAVA::Scene* scene);
    ~VisibilityCheckSystem();

    void RegisterEntity(DAVA::Entity* entity) override;
    void UnregisterEntity(DAVA::Entity* entity) override;
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void Recalculate();
    void Process(DAVA::float32 timeElapsed) override;
    void Draw();

    void InvalidateMaterials();

    void FixCurrentFrame();
    void ReleaseFixedFrame();

private:
    using EntityMap = DAVA::Map<DAVA::Entity*, DAVA::Vector<DAVA::Vector3>>;

    void BuildPointSetForEntity(EntityMap::value_type& item);
    void BuildIndexSet();
    DAVA::Color GetNormalizedColorForEntity(const EntityMap::value_type& item) const;

    DAVA::Camera* GetRenderCamera() const;
    DAVA::Camera* GetFinalGatherCamera() const;

    void UpdatePointSet();
    void Prerender();
    void CreateRenderTarget();

    bool CacheIsValid();
    void BuildCache();

    bool ShouldDrawRenderObject(DAVA::RenderObject*) override;

    struct StateCache
    {
        DAVA::Size2i viewportSize;
        DAVA::Matrix4 viewprojMatrix;
        DAVA::Camera* camera = nullptr;
    };

private:
    EntityMap entitiesWithVisibilityComponent;
    DAVA::Landscape* landscape = nullptr;
    DAVA::Vector<VisibilityCheckRenderer::VisbilityPoint> controlPoints;
    DAVA::Vector<DAVA::uint32> controlPointIndices;
    DAVA::Map<DAVA::RenderObject*, DAVA::Entity*> renderObjectToEntity;
    VisibilityCheckRenderer renderer;
    StateCache stateCache;
    size_t currentPointIndex = 0;
    bool shouldPrerender = true;
    bool forceRebuildPoints = true;
    bool shouldFixFrame = false;
};

#endif
