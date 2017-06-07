#include "Debug/DVAssert.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Lod/LodComponent.h"

#include "Scene/SceneEditor2.h"
#include "Scene/System/EditorStatisticsSystem.h"

#include "Classes/Selection/Selection.h"

using namespace DAVA;

struct TrianglesData
{
    Vector<uint32> storedTriangles;
    Vector<uint32> visibleTriangles;
    Vector<RenderObject*> renderObjects;
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
                int32 trianglesCount = pg->GetPrimitiveCount();
                triangles[lodIndex] += trianglesCount;
                if (batchIsVisible)
                {
                    visibleTriangles[lodIndex] += trianglesCount;
                }
            }
        }
    }
}

void EnumerateTriangles(TrianglesData& triangles)
{
    std::fill(triangles.storedTriangles.begin(), triangles.storedTriangles.end(), 0);
    std::fill(triangles.visibleTriangles.begin(), triangles.visibleTriangles.end(), 0);
    for (RenderObject* ro : triangles.renderObjects)
    {
        if (ro && (ro->GetType() == RenderObject::TYPE_MESH || ro->GetType() == RenderObject::TYPE_SPEED_TREE))
        {
            EnumerateTriangles(ro, triangles.storedTriangles, triangles.visibleTriangles);
        }
    }
}

void EnumerateRenderObjectsRecursive(Entity* entity, Vector<RenderObject*>& renderObjects, bool recursive)
{
    if (HasComponent(entity, Component::RENDER_COMPONENT))
    {
        uint32 componentsCount = entity->GetComponentCount(Component::RENDER_COMPONENT);
        for (uint32 c = 0; c < componentsCount; ++c)
        {
            RenderComponent* rc = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT, c));
            RenderObject* ro = rc->GetRenderObject();
            if (ro != nullptr)
            {
                if (std::find(renderObjects.begin(), renderObjects.end(), ro) == renderObjects.end())
                {
                    renderObjects.push_back(ro);
                }
            }
        }
    }

    if (recursive)
    {
        uint32 count = entity->GetChildrenCount();
        for (uint32 c = 0; c < count; ++c)
        {
            EnumerateRenderObjectsRecursive(entity->GetChild(c), renderObjects, recursive);
        }
    }
}

void EnumerateRenderObjects(const SelectableGroup& group, Vector<RenderObject*>& renderObjects)
{
    renderObjects.clear();
    if (group.IsEmpty())
        return;

    renderObjects.reserve(group.GetSize());

    const bool recursive = SettingsManager::GetValue(Settings::Internal_LODEditor_Recursive).AsBool();
    for (auto entity : group.ObjectsOfType<DAVA::Entity>())
    {
        EnumerateRenderObjectsRecursive(entity, renderObjects, recursive);
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
}

void EditorStatisticsSystem::RemoveComponent(Entity* entity, Component* component)
{
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
    auto CalculateTrianglesForMode = [this](eEditorMode mode)
    {
        Vector<uint32> storedTriangles = triangles[mode].storedTriangles;
        Vector<uint32> visibleTriangles = triangles[mode].visibleTriangles;

        EditorStatisticsSystemInternal::EnumerateTriangles(triangles[mode]);
        if (triangles[mode].storedTriangles != storedTriangles || triangles[mode].visibleTriangles != visibleTriangles)
        {
            EmitInvalidateUI(FLAG_TRIANGLES);
        }
    };

    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());

    //Scene
    triangles[eEditorMode::MODE_ALL_SCENE].renderObjects.clear();
    Camera* drawCamera = editorScene->GetDrawCamera();
    if (drawCamera != nullptr)
    {
        uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
        editorScene->renderSystem->GetRenderHierarchy()->Clip(drawCamera, triangles[eEditorMode::MODE_ALL_SCENE].renderObjects, currVisibilityCriteria);
    }
    CalculateTrianglesForMode(eEditorMode::MODE_ALL_SCENE);

    //Selection
    triangles[eEditorMode::MODE_SELECTION].renderObjects.clear();
    const SelectableGroup& selection = Selection::GetSelection();
    EditorStatisticsSystemInternal::EnumerateRenderObjects(selection, triangles[eEditorMode::MODE_SELECTION].renderObjects);
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
