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

void EditorVegetationSystem::AddEntity(DAVA::Entity* entity)
{
    DVASSERT(DAVA::HasComponent(entity, DAVA::Component::RENDER_COMPONENT));

    DAVA::VegetationRenderObject* vro = DAVA::GetVegetation(entity);
    if (vro != nullptr)
    {
        DVASSERT(std::find(vegetationObjects.begin(), vegetationObjects.end(), vro) == vegetationObjects.end());
        vegetationObjects.push_back(vro);
    }
}

void EditorVegetationSystem::RemoveEntity(DAVA::Entity* entity)
{
    DVASSERT(DAVA::HasComponent(entity, DAVA::Component::RENDER_COMPONENT));

    DAVA::VegetationRenderObject* vro = DAVA::GetVegetation(entity);
    if (vro != nullptr)
    {
        DVASSERT(std::find(vegetationObjects.begin(), vegetationObjects.end(), vro) != vegetationObjects.end());
        DAVA::FindAndRemoveExchangingWithLast(vegetationObjects, vro);
    }
}

void EditorVegetationSystem::GetActiveVegetation(DAVA::Vector<DAVA::VegetationRenderObject*>& activeVegetationObjects)
{
    static const DAVA::uint32 VISIBILITY_CRITERIA = DAVA::RenderObject::VISIBLE | DAVA::RenderObject::VISIBLE_QUALITY;

    for (DAVA::VegetationRenderObject* ro : vegetationObjects)
    {
        if ((ro->GetFlags() & VISIBILITY_CRITERIA) == VISIBILITY_CRITERIA)
        {
            activeVegetationObjects.push_back(ro);
        }
    }
}

void EditorVegetationSystem::ReloadVegetation()
{
    for (DAVA::VegetationRenderObject* ro : vegetationObjects)
    {
        ro->Rebuild();
    }
}
