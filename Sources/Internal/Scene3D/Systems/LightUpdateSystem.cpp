#include "Scene3D/Systems/LightUpdateSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"

#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Scene.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
LightUpdateSystem::LightUpdateSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void LightUpdateSystem::Process(float32 timeElapsed)
{
    TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (Entity* entity : tsc->worldTransformChanged)
    {
        RecalcLight(entity);
    }
}

void LightUpdateSystem::RecalcLight(Entity* entity)
{
    // Update new transform pointer, and mark that transform is changed
    Matrix4* worldTransformPointer = (static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
    Light* light = (static_cast<LightComponent*>(entity->GetComponent(Component::LIGHT_COMPONENT)))->GetLightObject();
    light->SetPositionDirectionFromMatrix(*worldTransformPointer);
    entity->GetScene()->renderSystem->MarkForUpdate(light);
}

void LightUpdateSystem::AddEntity(Entity* entity)
{
    Light* lightObject = (static_cast<LightComponent*>(entity->GetComponent(Component::LIGHT_COMPONENT)))->GetLightObject();
    if (!lightObject)
        return;

    entityObjectMap.insert(entity, lightObject);
    GetScene()->GetRenderSystem()->AddLight(lightObject);

    RecalcLight(entity);
}

void LightUpdateSystem::RemoveEntity(Entity* entity)
{
    Light* lightObject = entityObjectMap.at(entity);
    if (!lightObject)
    {
        return;
    }

    GetScene()->GetRenderSystem()->RemoveLight(lightObject);
    entityObjectMap.erase(entity);
}
};