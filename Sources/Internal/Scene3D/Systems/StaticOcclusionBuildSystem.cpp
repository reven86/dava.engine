#include "Scene3D/Systems/StaticOcclusionBuildSystem.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Sound/SoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Lod/LodSystem.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Landscape.h"

namespace DAVA
{
StaticOcclusionBuildSystem::StaticOcclusionBuildSystem(Scene* scene)
    : SceneSystem(scene)
{
    staticOcclusion = 0;
    activeIndex = -1;
    componentInProgress = 0;
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
}

StaticOcclusionBuildSystem::~StaticOcclusionBuildSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
    SafeDelete(staticOcclusion);
}

void StaticOcclusionBuildSystem::AddEntity(Entity* entity)
{
    occlusionEntities.push_back(entity);
}

void StaticOcclusionBuildSystem::RemoveEntity(Entity* entity)
{
    occlusionEntities.erase(std::remove(occlusionEntities.begin(), occlusionEntities.end(), entity), occlusionEntities.end());
}

void StaticOcclusionBuildSystem::ImmediateEvent(Component* _component, uint32 event)
{
    Entity* entity = _component->GetEntity();
    StaticOcclusionComponent* component = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    if (component->GetPlaceOnLandscape() && ((event == EventSystem::WORLD_TRANSFORM_CHANGED) || (event == EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED)))
    {
        component->cellHeightOffset.clear();
        component->cellHeightOffset.resize(component->GetSubdivisionsX() * component->GetSubdivisionsY(), 0);
        /*place on landscape*/
        Landscape* landscape = FindLandscape(GetScene());
        AABBox3 localBox = component->GetBoundingBox();
        Vector3 boxSize = localBox.GetSize();
        AABBox3 bbox;
        localBox.GetTransformedBox(GetTransformComponent(entity)->GetWorldTransform(), bbox);
        uint32 xSubdivisions = component->GetSubdivisionsX();
        uint32 ySubdivisions = component->GetSubdivisionsY();
        boxSize.x /= xSubdivisions;
        boxSize.y /= ySubdivisions;

        if (landscape)
        {
            //place on landscape
            for (uint32 xs = 0; xs < xSubdivisions; ++xs)
                for (uint32 ys = 0; ys < ySubdivisions; ++ys)
                {
                    Vector3 v = bbox.min + Vector3(boxSize.x * (xs + 0.5f), boxSize.y * (ys + 0.5f), 0);
                    if (landscape->PlacePoint(v, v))
                        component->cellHeightOffset[xs + ys * xSubdivisions] = v.z - bbox.min.z;
                }
        }
    }
}

void StaticOcclusionBuildSystem::PrepareRenderObjects()
{
    GetScene()->staticOcclusionSystem->ClearOcclusionObjects();
    landscape = nullptr;

    // Prepare render objects
    Vector<Entity*> sceneEntities;
    CollectEntitiesForOcclusionRecursively(sceneEntities, GetScene());
    objectsCount = static_cast<uint32>(sceneEntities.size());

    uint16 index = 0;
    for (uint32 k = 0; k < objectsCount; ++k)
    {
        RenderObject* renderObject = GetRenderObject(sceneEntities[k]);
        auto renderObjectType = renderObject->GetType();
        if ((RenderObject::TYPE_MESH == renderObjectType) || (RenderObject::TYPE_SPEED_TREE == renderObjectType))
        {
            renderObject->AddFlag(RenderObject::VISIBLE_STATIC_OCCLUSION);
            renderObject->SetStaticOcclusionIndex(index++);
        }
        else if (RenderObject::TYPE_LANDSCAPE == renderObjectType)
        {
            landscape = static_cast<Landscape*>(renderObject);
        }
    }
}

void StaticOcclusionBuildSystem::Build()
{
    if (occlusionEntities.empty())
        return;

    activeIndex = 0;

    if (nullptr == staticOcclusion)
        staticOcclusion = new StaticOcclusion();

    PrepareRenderObjects();
    StartBuildOcclusion();
}

void StaticOcclusionBuildSystem::Cancel()
{
    activeIndex = -1;
    SafeDelete(staticOcclusion);

    GetScene()->staticOcclusionSystem->InvalidateOcclusion();
    SceneForceLod(LodComponent::INVALID_LOD_LAYER);
}

void StaticOcclusionBuildSystem::CollectEntitiesForOcclusionRecursively(Vector<Entity*>& dest, Entity* entity)
{
    if (GetAnimationComponent(entity)) //skip animated hierarchies
        return;

    if (GetRenderComponent(entity))
        dest.push_back(entity);

    for (int32 i = 0, sz = entity->GetChildrenCount(); i < sz; ++i)
        CollectEntitiesForOcclusionRecursively(dest, entity->GetChild(i));
}

void StaticOcclusionBuildSystem::StartBuildOcclusion()
{
    if (activeIndex == static_cast<uint32>(-1))
        return; // System inactive

    //global preparations
    SetCamera(GetScene()->GetCurrentCamera());

    SceneForceLod(0);

    // Prepare occlusion per component
    Entity* entity = occlusionEntities[activeIndex];

    componentInProgress = static_cast<StaticOcclusionDataComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT));
    if (componentInProgress)
    {
        // We detach component from system, to let system know that this data is not valid right now.
        // Entity will be removed from system that apply occlusion information.
        entity->DetachComponent(componentInProgress);
    }
    else
    {
        componentInProgress = new StaticOcclusionDataComponent();
    }
    StaticOcclusionData& data = componentInProgress->GetData();

    StaticOcclusionComponent* occlusionComponent = static_cast<StaticOcclusionComponent*>(entity->GetComponent(Component::STATIC_OCCLUSION_COMPONENT));
    TransformComponent* transformComponent = static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
    AABBox3 localBox = occlusionComponent->GetBoundingBox();
    AABBox3 worldBox;
    localBox.GetTransformedBox(transformComponent->GetWorldTransform(), worldBox);

    data.Init(occlusionComponent->GetSubdivisionsX(), occlusionComponent->GetSubdivisionsY(),
              occlusionComponent->GetSubdivisionsZ(), objectsCount, worldBox, occlusionComponent->GetCellHeightOffsets());

    if (nullptr == staticOcclusion)
        staticOcclusion = new StaticOcclusion();

    staticOcclusion->StartBuildOcclusion(&data, GetScene()->GetRenderSystem(), landscape, occlusionComponent->GetOcclusionPixelThreshold(), occlusionComponent->GetOcclusionPixelThresholdForSpeedtree());
}

void StaticOcclusionBuildSystem::FinishBuildOcclusion()
{
    Component* prevComponent = occlusionEntities[activeIndex]->GetComponent(Component::STATIC_OCCLUSION_DATA_COMPONENT);

    // We've detached component so we verify that here we still do not have this component.
    DVASSERT(prevComponent == 0);

    occlusionEntities[activeIndex]->AddComponent(componentInProgress);
    componentInProgress = 0;

    activeIndex++;
    if (activeIndex == occlusionEntities.size())
    {
        activeIndex = -1;
    }
    else
    {
        // not final index add more occlusion build cycle
        StartBuildOcclusion();
        return;
    }

    SceneForceLod(LodComponent::INVALID_LOD_LAYER);

    Scene* scene = GetScene();
    scene->staticOcclusionSystem->CollectOcclusionObjectsRecursively(scene);
}

bool StaticOcclusionBuildSystem::IsInBuild() const
{
    return (static_cast<uint32>(-1) != activeIndex);
}

uint32 StaticOcclusionBuildSystem::GetBuildStatus() const
{
    uint32 ret = 0;

    if (staticOcclusion)
    {
        uint32 currentStepsCount = staticOcclusion->GetCurrentStepsCount();
        uint32 totalStepsCount = staticOcclusion->GetTotalStepsCount();
        ret = (currentStepsCount * 100) / totalStepsCount;
    }
    return ret;
}

const String& StaticOcclusionBuildSystem::GetBuildStatusInfo() const
{
    static const String defaultMessage = "Static occlusion system not started";
    if (staticOcclusion == nullptr)
    {
        return defaultMessage;
    }

    return staticOcclusion->GetInfoMessage();
}

void StaticOcclusionBuildSystem::SceneForceLod(int32 forceLodIndex)
{
    Vector<Entity*> lodEntities;
    GetScene()->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
    uint32 size = static_cast<uint32>(lodEntities.size());
    for (uint32 k = 0; k < size; ++k)
    {
        LodComponent* lodComponent = static_cast<LodComponent*>(lodEntities[k]->GetComponent(Component::LOD_COMPONENT));
        GetScene()->lodSystem->SetForceLodLayer(lodComponent, forceLodIndex);
    }
    GetScene()->lodSystem->Process(0.0f);
}

void StaticOcclusionBuildSystem::Process(float32 timeElapsed)
{
    if (activeIndex == static_cast<uint32>(-1))
        return;

    bool finished = staticOcclusion->ProccessBlock();
    if (finished)
    {
        FinishBuildOcclusion();
    }
}
};
