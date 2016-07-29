#include "Scene/System/EditorVegetationSystem.h"

#include "Entity/Component.h"

#include "Debug/DVAssert.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Scene/SceneEditor2.h"


EditorVegetationSystem::EditorVegetationSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

EditorVegetationSystem::~EditorVegetationSystem() = default;

void EditorVegetationSystem::AddEntity(DAVA::Entity* entity)
{
    DVASSERT(DAVA::HasComponent(entity, DAVA::Component::RENDER_COMPONENT));

    DAVA::VegetationRenderObject *vro = DAVA::GetVegetation(entity);
    if (vro != nullptr)
    {
        DVASSERT(std::find(vegetationObjects.begin(), vegetationObjects.end(), vro) == vegetationObjects.end());
        vegetationObjects.push_back(vro);
    }
}

void EditorVegetationSystem::RemoveEntity(DAVA::Entity* entity)
{
    DVASSERT(DAVA::HasComponent(entity, DAVA::Component::RENDER_COMPONENT));

    DAVA::VegetationRenderObject *vro = DAVA::GetVegetation(entity);
    if (vro != nullptr)
    {
        DVASSERT(std::find(vegetationObjects.begin(), vegetationObjects.end(), vro) != vegetationObjects.end());
        DAVA::FindAndRemoveExchangingWithLast(vegetationObjects, vro);
    }
}

void EditorVegetationSystem::GetActiveVegetation(DAVA::Vector<DAVA::VegetationRenderObject*>& activeVegetationObjects)
{
    for (DAVA::VegetationRenderObject *ro : vegetationObjects)
    {
        if (ro->GetFlags() & DAVA::RenderObject::VISIBLE_QUALITY)
        {
            activeVegetationObjects.push_back(ro);
        }
    }
}

