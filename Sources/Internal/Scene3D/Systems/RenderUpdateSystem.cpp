#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"

#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Scene.h"
#include "Platform/SystemTimer.h"
#include "Debug/CPUProfiler.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
RenderUpdateSystem::RenderUpdateSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

RenderUpdateSystem::~RenderUpdateSystem()
{
}

void RenderUpdateSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        Entity* entity = component->GetEntity();
        // Update new transform pointer, and mark that transform is changed
        Matrix4* worldTransformPointer = (static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
        RenderObject* object = (static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT)))->GetRenderObject();
        if (nullptr != object)
        {
            object->SetWorldTransformPtr(worldTransformPointer);
            entity->GetScene()->renderSystem->MarkForUpdate(object);
        }
    }
}

void RenderUpdateSystem::AddEntity(Entity* entity)
{
    RenderObject* renderObject = (static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT)))->GetRenderObject();
    if (!renderObject)
        return;
    Matrix4* worldTransformPointer = (static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
    renderObject->SetWorldTransformPtr(worldTransformPointer);
    UpdateActiveIndexes(entity, renderObject);
    entityObjectMap.insert(entity, renderObject);
    GetScene()->GetRenderSystem()->RenderPermanent(renderObject);
}

void RenderUpdateSystem::RemoveEntity(Entity* entity)
{
    RenderObject* renderObject = entityObjectMap.at(entity);
    if (!renderObject)
    {
        return;
    }

    GetScene()->GetRenderSystem()->RemoveFromRender(renderObject);

    entityObjectMap.erase(entity);
}

void RenderUpdateSystem::Process(float32 timeElapsed)
{
    DAVA_CPU_PROFILER_SCOPE(CPUMarkerName::SCENE_RENDER_UPDATE_SYSTEM);

    RenderSystem* renderSystem = GetScene()->GetRenderSystem();
    renderSystem->SetMainCamera(GetScene()->GetCurrentCamera());
    renderSystem->SetDrawCamera(GetScene()->GetDrawCamera());

    GetScene()->GetRenderSystem()->Update(timeElapsed);
}

void RenderUpdateSystem::UpdateActiveIndexes(Entity* entity, RenderObject* object)
{
    Entity* parent;

    // search effective lod index
    parent = entity;
    while (nullptr != parent)
    {
        LodComponent* lc = GetLodComponent(parent);
        if (nullptr != lc)
        {
            object->SetLodIndex(lc->GetCurrentLod());
            break;
        }

        parent = parent->GetParent();
    }

    // search effective switch index
    parent = entity;
    while (nullptr != parent)
    {
        SwitchComponent* sc = GetSwitchComponent(parent);
        if (nullptr != sc)
        {
            object->SetSwitchIndex(sc->GetSwitchIndex());
            break;
        }

        parent = parent->GetParent();
    }
}
};