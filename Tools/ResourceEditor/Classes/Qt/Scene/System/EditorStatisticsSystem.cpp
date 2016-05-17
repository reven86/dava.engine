#include "Debug/DVAssert.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"

#include "Scene/SceneEditor2.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/EditorStatisticsSystem.h"

using namespace DAVA;

struct TrianglesData
{
    Vector<uint32> storedTriangles;
    Vector<uint32> visibleTriangles;
    Vector<uint32> temporaryTriangles;
    Vector<RenderComponent*> renderComponents;
};

namespace EditorStatisticsSystemInternal
{
static const int32 SIZE_OF_TRIANGLES = LodComponent::MAX_LOD_LAYERS + 1;

void EnumerateTriangles(RenderObject* renderObject, Vector<uint32>& triangles, Vector<uint32>& visibleTriangles)
{
    uint32 batchCount = renderObject->GetRenderBatchCount();
    for (uint32 b = 0; b < batchCount; ++b)
    {
        int32 lodIndex = 0;
        int32 switchIndex = 0;

        RenderBatch* rb = renderObject->GetRenderBatch(b, lodIndex, switchIndex);
        lodIndex += 1; //because of non-lod index is -1
        if (lodIndex < 0)
        {
            continue; //means that lod is uninitialized
        }
        DVASSERT(lodIndex <= static_cast<int32>(triangles.size()));

        if (IsPointerToExactClass<RenderBatch>(rb))
        {
            bool batchIsVisible = false;
            uint32 activeBatchCount = renderObject->GetActiveRenderBatchCount();
            for (uint32 a = 0; a < activeBatchCount; ++a)
            {
                if (renderObject->GetActiveRenderBatch(a) == rb)
                {
                    batchIsVisible = true;
                    break;
                }
            }

            PolygonGroup* pg = rb->GetPolygonGroup();
            if (nullptr != pg)
            {
                int32 indexCount = pg->GetIndexCount() / 3;
                triangles[lodIndex] += indexCount;
                if (batchIsVisible)
                {
                    visibleTriangles[lodIndex] += indexCount;
                }
            }
        }
    }
}

void EnumerateTriangles(TrianglesData& triangles)
{
    std::fill(triangles.temporaryTriangles.begin(), triangles.temporaryTriangles.end(), 0);
    std::fill(triangles.visibleTriangles.begin(), triangles.visibleTriangles.end(), 0);
    for (auto& rc : triangles.renderComponents)
    {
        RenderObject* ro = rc->GetRenderObject();
        if (ro && (ro->GetType() == RenderObject::TYPE_MESH || ro->GetType() == RenderObject::TYPE_SPEED_TREE))
        {
            EnumerateTriangles(ro, triangles.temporaryTriangles, triangles.visibleTriangles);
        }
    }
}

void EnumerateRenderComponentsRecursive(Entity* entity, Vector<RenderComponent*>& renderComponents, bool recursive)
{
    if (HasComponent(entity, Component::RENDER_COMPONENT))
    {
        uint32 componentsCount = entity->GetComponentCount(Component::RENDER_COMPONENT);
        for (uint32 c = 0; c < componentsCount; ++c)
        {
            RenderComponent* rc = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT, c));
            DVASSERT(std::find(renderComponents.begin(), renderComponents.end(), rc) == renderComponents.end());

            renderComponents.push_back(rc);
        }
    }

    if (recursive)
    {
        uint32 count = entity->GetChildrenCount();
        for (uint32 c = 0; c < count; ++c)
        {
            EnumerateRenderComponentsRecursive(entity->GetChild(c), renderComponents, recursive);
        }
    }
}

void EnumerateRenderComponents(const SelectableGroup& group, Vector<RenderComponent*>& renderComponents)
{
    renderComponents.clear();
    if (group.IsEmpty())
        return;

    renderComponents.reserve(group.GetSize());

    const bool ignoreChildren = SettingsManager::GetValue(Settings::Scene_RefreshLodForNonSolid).AsBool();
    for (auto entity : group.ObjectsOfType<DAVA::Entity>())
    {
        bool recursive = entity->GetSolid() || !ignoreChildren;
        EnumerateRenderComponentsRecursive(entity, renderComponents, recursive);
    }
}
}
EditorStatisticsSystem::EditorStatisticsSystem(Scene* scene)
    : SceneSystem(scene)
{
    triangles.resize(eEditorMode::MODE_COUNT);
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        triangles[m].storedTriangles.resize(EditorStatisticsSystemInternal::SIZE_OF_TRIANGLES, 0);
        triangles[m].visibleTriangles.resize(EditorStatisticsSystemInternal::SIZE_OF_TRIANGLES, 0);
        triangles[m].temporaryTriangles.resize(EditorStatisticsSystemInternal::SIZE_OF_TRIANGLES, 0);
    }
}

void EditorStatisticsSystem::AddEntity(Entity* entity)
{
    if (HasComponent(entity, Component::RENDER_COMPONENT))
    {
        AddComponent(entity, GetRenderComponent(entity));
    }
}

void EditorStatisticsSystem::RemoveEntity(Entity* entity)
{
    if (HasComponent(entity, Component::RENDER_COMPONENT))
    {
        RemoveComponent(entity, GetRenderComponent(entity));
    }
}

void EditorStatisticsSystem::AddComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Component::RENDER_COMPONENT)
    {
        Vector<RenderComponent*>& renderComponents = triangles[eEditorMode::MODE_ALL_SCENE].renderComponents;

        RenderComponent* newComponent = static_cast<RenderComponent*>(component);
        DVASSERT(std::find(renderComponents.begin(), renderComponents.end(), newComponent) == renderComponents.end());
        renderComponents.push_back(newComponent);
    }
}

void EditorStatisticsSystem::RemoveComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Component::RENDER_COMPONENT)
    {
        RenderComponent* removedComponent = static_cast<RenderComponent*>(component);
        FindAndRemoveExchangingWithLast(triangles[eEditorMode::MODE_ALL_SCENE].renderComponents, removedComponent);
    }
}

const Vector<uint32>& EditorStatisticsSystem::GetTriangles(eEditorMode mode, bool allTriangles) const
{
    if (allTriangles)
    {
        return triangles[mode].storedTriangles;
    }

    return triangles[mode].visibleTriangles;
}

void EditorStatisticsSystem::Process(float32 timeElapsed)
{
    CalculateTriangles();
    DispatchSignals();
}

void EditorStatisticsSystem::CalculateTriangles()
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());

    auto CalculateTrianglesForMode = [this](eEditorMode mode) {
        EditorStatisticsSystemInternal::EnumerateTriangles(triangles[mode]);
        if (triangles[mode].storedTriangles != triangles[mode].temporaryTriangles)
        {
            triangles[mode].storedTriangles.swap(triangles[mode].temporaryTriangles);
            EmitInvalidateUI(FLAG_TRIANGLES);
        }
    };

    //Scene
    CalculateTrianglesForMode(eEditorMode::MODE_ALL_SCENE);

    //Selection
    triangles[eEditorMode::MODE_SELECTION].renderComponents.clear();
    EditorStatisticsSystemInternal::EnumerateRenderComponents(editorScene->selectionSystem->GetSelection(), triangles[eEditorMode::MODE_SELECTION].renderComponents);
    CalculateTrianglesForMode(eEditorMode::MODE_SELECTION);
}

void EditorStatisticsSystem::EmitInvalidateUI(uint32 flags)
{
    invalidateUIflag = flags;
}

void EditorStatisticsSystem::DispatchSignals()
{
    if (invalidateUIflag == FLAG_NONE)
    {
        return;
    }

    for (auto& d : uiDelegates)
    {
        if (invalidateUIflag & FLAG_TRIANGLES)
        {
            d->UpdateTrianglesUI(this);
        }
    }

    invalidateUIflag = FLAG_NONE;
}

void EditorStatisticsSystem::AddDelegate(EditorStatisticsSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    uiDelegates.push_back(uiDelegate);
    if (uiDelegate != nullptr)
    {
        uiDelegate->UpdateTrianglesUI(this);
    }
}

void EditorStatisticsSystem::RemoveDelegate(EditorStatisticsSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    FindAndRemoveExchangingWithLast(uiDelegates, uiDelegate);
}
