#include "Classes/Qt/Scene/System/WayEditSystem.h"
#include "Classes/Qt/Scene/System/PathSystem.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Commands2/WayEditCommands.h"
#include "Classes/Commands2/Base/RECommandNotificationObject.h"
#include "Classes/Settings/SettingsManager.h"

#include "Classes/Selection/Selection.h"

#include <Base/Singleton.h>
#include <Debug/DVAssert.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Math/AABBox3.h>
#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <Scene3D/Components/Waypoint/WaypointComponent.h>
#include <Utils/Utils.h>

namespace WayEditSystemDetail
{
struct AccessibleQueryParams
{
    DAVA::PathComponent::Waypoint* startPoint = nullptr;
    DAVA::PathComponent::Waypoint* destinationPoint = nullptr;
    DAVA::PathComponent::Waypoint* excludePoint = nullptr;
    DAVA::PathComponent::Edge* excludeEdge = nullptr;
};

bool IsAccessibleImpl(const AccessibleQueryParams& params, DAVA::Set<DAVA::PathComponent::Waypoint*>& passedPoints)
{
    DVASSERT(params.startPoint != nullptr);
    DVASSERT(params.destinationPoint != nullptr);
    if (params.startPoint == params.destinationPoint)
    {
        return true;
    }

    for (DAVA::PathComponent::Edge* edge : params.startPoint->edges)
    {
        if (edge == params.excludeEdge || edge->destination == params.excludePoint)
        {
            continue;
        }

        if (passedPoints.insert(edge->destination).second == false)
        {
            continue;
        }

        AccessibleQueryParams deeperParams = params;
        deeperParams.startPoint = edge->destination;
        if (IsAccessibleImpl(deeperParams, passedPoints) == true)
        {
            return true;
        }
    }

    return false;
}

bool IsAccessible(const AccessibleQueryParams& params)
{
    DAVA::Set<DAVA::PathComponent::Waypoint*> passedPoints;
    return IsAccessibleImpl(params, passedPoints);
}
}

WayEditSystem::WayEditSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

void WayEditSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    if (event == DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED)
    {
        DAVA::Entity* parentEntity = component->GetEntity();
        DAVA::WaypointComponent* waypoint = DAVA::GetWaypointComponent(parentEntity);
        if (waypoint != nullptr)
        {
            waypoint->GetWaypoint()->position = parentEntity->GetLocalTransform().GetTranslationVector();
        }
    }
}

void WayEditSystem::AddEntity(DAVA::Entity* newWaypoint)
{
    waypointEntities.push_back(newWaypoint);
}

void WayEditSystem::RemoveEntity(DAVA::Entity* removedPoint)
{
    DAVA::FindAndRemoveExchangingWithLast(waypointEntities, removedPoint);
    DAVA::WaypointComponent* waypointComponent = GetWaypointComponent(removedPoint);
    if (waypointComponent == nullptr)
    {
        return;
    }

    DAVA::PathComponent::Waypoint* waypoint = waypointComponent->GetWaypoint();

    if (currentSelection.ContainsObject(removedPoint))
    {
        currentSelection.Remove(removedPoint);
    }

    DAVA::FindAndRemoveExchangingWithLast(selectedWaypoints, waypoint);
    DAVA::FindAndRemoveExchangingWithLast(prevSelectedWaypoints, waypoint);
}

bool WayEditSystem::HasCustomRemovingForEntity(DAVA::Entity* entityToRemove) const
{
    if (IsWayEditEnabled() == false)
    {
        DVASSERT(entityToRemove->GetComponentCount(DAVA::Component::WAYPOINT_COMPONENT) == 0);
        DVASSERT(entityToRemove->GetComponentCount(DAVA::Component::EDGE_COMPONENT) == 0);
        return false;
    }

    return GetWaypointComponent(entityToRemove) != nullptr;
}

void WayEditSystem::PerformRemoving(DAVA::Entity* entityToRemove)
{
    using namespace DAVA;
    DVASSERT(IsWayEditEnabled() == true);

    WaypointComponent* waypointComponent = GetWaypointComponent(entityToRemove);
    DVASSERT(waypointComponent != nullptr);
    DVASSERT(waypointComponent->GetWaypoint()->IsStarting() == false);

    PathComponent* path = waypointComponent->GetPath();
    PathComponent::Waypoint* waypointToRemove = waypointComponent->GetWaypoint();
// lookup path that point was removed from
#if defined(__DAVAENGINE_DEBUG__)
    Entity* parentEntity = entityToRemove->GetParent();
    bool pathFound = false;
    DVASSERT(path == GetPathComponent(parentEntity));

    bool waypointFound = false;
    for (PathComponent::Waypoint* waypoint : path->GetPoints())
    {
        if (waypoint == waypointToRemove)
        {
            waypointFound = true;
            break;
        }
    }
    DVASSERT(waypointFound);
#endif

    Vector<std::pair<PathComponent::Waypoint*, PathComponent::Edge*>> edgesToRemovedPoint;

    const Vector<PathComponent::Waypoint*>& pathPoints = path->GetPoints();
    for (PathComponent::Waypoint* checkWaypoint : pathPoints)
    {
        if (checkWaypoint == waypointToRemove)
        {
            continue;
        }

        for (PathComponent::Edge* edge : checkWaypoint->edges)
        {
            if (edge->destination == waypointToRemove)
            {
                edgesToRemovedPoint.push_back(std::make_pair(checkWaypoint, edge));
            }
        }
    }

    PathComponent::Waypoint* startPoint = path->GetStartWaypoint();
    DVASSERT(startPoint != nullptr);

    WayEditSystemDetail::AccessibleQueryParams params;
    params.startPoint = startPoint;
    params.excludePoint = waypointToRemove;

    Vector<PathComponent::Waypoint*> inaccessiblePoints;
    for (PathComponent::Edge* edge : waypointToRemove->edges)
    {
        params.destinationPoint = edge->destination;
        if (WayEditSystemDetail::IsAccessible(params) == false)
        {
            inaccessiblePoints.push_back(edge->destination);
        }
    }

    SceneEditor2* sceneEditor = GetSceneEditor();
    for (auto& edgeNode : edgesToRemovedPoint)
    {
        sceneEditor->Exec(std::make_unique<RemoveEdgeCommand>(sceneEditor, path, edgeNode.first, edgeNode.second));
    }

    Vector<PathComponent::Edge*> edgesToRemove = waypointToRemove->edges;
    for (PathComponent::Edge* e : edgesToRemove)
    {
        sceneEditor->Exec(std::make_unique<RemoveEdgeCommand>(sceneEditor, path, waypointToRemove, e));
    }

    sceneEditor->Exec(std::make_unique<RemoveWaypointCommand>(sceneEditor, path, waypointToRemove));

    for (PathComponent::Waypoint* inaccessiblePoint : inaccessiblePoints)
    {
        for (auto& srcNode : edgesToRemovedPoint)
        {
            if (srcNode.first != inaccessiblePoint)
            {
                PathComponent::Edge* newEdge = new PathComponent::Edge();
                newEdge->destination = inaccessiblePoint;
                sceneEditor->Exec(std::make_unique<AddEdgeCommand>(sceneEditor, path, srcNode.first, newEdge));
            }
        }
    }
}

void WayEditSystem::SetScene(DAVA::Scene* scene)
{
    {
        DAVA::Scene* currentScene = GetScene();
        if (currentScene != nullptr)
        {
            currentScene->GetEventSystem()->UnregisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
        }
    }

    SceneSystem::SetScene(scene);

    {
        DAVA::Scene* currentScene = GetScene();
        if (currentScene != nullptr)
        {
            currentScene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
        }
    }
}

void WayEditSystem::Process(DAVA::float32 timeElapsed)
{
}

void WayEditSystem::ResetSelection()
{
    selectedWaypoints.clear();
    prevSelectedWaypoints.clear();
    underCursorPathEntity = nullptr;
}

void WayEditSystem::ProcessSelection(const SelectableGroup& selection)
{
    prevSelectedWaypoints = selectedWaypoints;
    selectedWaypoints.clear();

    if (currentSelection != selection)
    {
        currentSelection = selection;

        for (auto entity : currentSelection.ObjectsOfType<DAVA::Entity>())
        {
            DAVA::WaypointComponent* waypointComponent = GetWaypointComponent(entity);
            if (waypointComponent != nullptr && GetPathComponent(entity->GetParent()))
            {
                selectedWaypoints.push_back(waypointComponent->GetWaypoint());
            }
        }
    }
}

bool WayEditSystem::Input(DAVA::UIEvent* event)
{
    using namespace DAVA;

    SceneEditor2* sceneEditor = GetSceneEditor();
    if (isEnabled && (eMouseButtons::LEFT == event->mouseButton))
    {
        if (UIEvent::Phase::MOVE == event->phase)
        {
            underCursorPathEntity = nullptr;
            const SelectableGroup::CollectionType& collObjects = sceneEditor->collisionSystem->ObjectsRayTestFromCamera();
            if (collObjects.size() == 1)
            {
                Entity* firstEntity = collObjects.front().AsEntity();
                if ((firstEntity != nullptr) && GetWaypointComponent(firstEntity) && GetPathComponent(firstEntity->GetParent()))
                {
                    underCursorPathEntity = GetWaypointComponent(firstEntity)->GetWaypoint();
                }
            }
        }
        else if (UIEvent::Phase::BEGAN == event->phase)
        {
            inCloneState = sceneEditor->modifSystem->InCloneState();
        }
        else if (UIEvent::Phase::ENDED == event->phase)
        {
            bool cloneJustDone = false;
            if (inCloneState && !sceneEditor->modifSystem->InCloneState())
            {
                inCloneState = false;
                cloneJustDone = true;
            }

            ProcessSelection(Selection::GetSelection());

            const auto& keyboard = InputSystem::Instance()->GetKeyboard();
            bool shiftPressed = keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::RSHIFT);

            if (!shiftPressed)
            {
                // we need to use shift key to add waypoint or edge
                return false;
            }

            Entity* currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
            if (currentWayParent == nullptr)
            {
                // we need to have entity with path component
                return false;
            }

            PathComponent* pathComponent = GetPathComponent(currentWayParent);
            Vector<PathComponent::Waypoint*> validPrevPoints;
            FilterSelection(pathComponent, prevSelectedWaypoints, validPrevPoints);

            if (selectedWaypoints.empty() && cloneJustDone == false)
            {
                Vector3 lanscapeIntersectionPos;
                bool lanscapeIntersected = sceneEditor->collisionSystem->LandRayTestFromCamera(lanscapeIntersectionPos);

                // add new waypoint on the landscape
                if (lanscapeIntersected)
                {
                    if (validPrevPoints.empty())
                    {
                        if (pathComponent->GetPoints().size() > 0)
                        {
                            // current path has waypoints but none of them was selected. Point adding is denied
                            return false;
                        }
                    }

                    Matrix4 parentTransform = currentWayParent->GetWorldTransform();
                    parentTransform.Inverse();

                    Matrix4 waypointTransform;
                    waypointTransform.SetTranslationVector(lanscapeIntersectionPos);
                    waypointTransform = waypointTransform * parentTransform;

                    int32 waypointsCount = static_cast<int32>(pathComponent->GetPoints().size());
                    PathComponent::Waypoint* waypoint = new PathComponent::Waypoint();
                    waypoint->name = FastName(Format("Waypoint_%d", waypointsCount));
                    waypoint->SetStarting(waypointsCount == 0);
                    waypoint->position = waypointTransform.GetTranslationVector();

                    Selection::Lock();
                    sceneEditor->BeginBatch("Add Waypoint", static_cast<uint32>(1 + validPrevPoints.size()));
                    sceneEditor->Exec(std::make_unique<AddWaypointCommand>(sceneEditor, pathComponent, waypoint));
                    if (!validPrevPoints.empty())
                    {
                        AddEdges(pathComponent, validPrevPoints, waypoint);
                    }
                    sceneEditor->EndBatch();

                    selectedWaypoints.clear();
                    selectedWaypoints.push_back(waypoint);
                    Selection::Unlock();
                }
            }
            else if ((selectedWaypoints.size() == 1) && (cloneJustDone == false))
            {
                Vector<PathComponent::Waypoint*> filteredSelection;
                FilterSelection(pathComponent, selectedWaypoints, filteredSelection);
                if (filteredSelection.empty() == false)
                {
                    PathComponent::Waypoint* nextWaypoint = selectedWaypoints.front();
                    Vector<PathComponent::Waypoint*> srcPointToAddEdges;
                    Vector<PathComponent::Waypoint*> srcPointToRemoveEdges;
                    DefineAddOrRemoveEdges(validPrevPoints, nextWaypoint, srcPointToAddEdges, srcPointToRemoveEdges);
                    size_t totalOperations = srcPointToAddEdges.size() + srcPointToRemoveEdges.size();
                    if (totalOperations > 0)
                    {
                        sceneEditor->BeginBatch(DAVA::Format("Add/Remove edges pointed on entity %s", nextWaypoint->name.c_str()), static_cast<DAVA::uint32>(totalOperations));
                        AddEdges(pathComponent, srcPointToAddEdges, nextWaypoint);
                        RemoveEdges(pathComponent, srcPointToRemoveEdges, nextWaypoint);
                        sceneEditor->EndBatch();
                    }
                }
            }
        }
    }
    return false;
}

void WayEditSystem::FilterSelection(DAVA::PathComponent* path, const DAVA::Vector<DAVA::PathComponent::Waypoint*>& srcPoints, DAVA::Vector<DAVA::PathComponent::Waypoint*>& validPoints)
{
    const DAVA::Vector<DAVA::PathComponent::Waypoint*>& points = path->GetPoints();
    DAVA::Set<DAVA::PathComponent::Waypoint*> pathPoints(points.begin(), points.end());

    validPoints.reserve(srcPoints.size());
    for (DAVA::PathComponent::Waypoint* waypoint : srcPoints)
    {
        if (pathPoints.count(waypoint))
        {
            validPoints.push_back(waypoint);
        }
    }
}

void WayEditSystem::DefineAddOrRemoveEdges(const DAVA::Vector<DAVA::PathComponent::Waypoint*>& srcPoints, DAVA::PathComponent::Waypoint* dstPoint,
                                           DAVA::Vector<DAVA::PathComponent::Waypoint*>& toAddEdge, DAVA::Vector<DAVA::PathComponent::Waypoint*>& toRemoveEdge)
{
    using namespace DAVA;
    for (PathComponent::Waypoint* srcPoint : srcPoints)
    {
        if (srcPoint == dstPoint)
        {
            continue;
        }

        bool edgeFound = false;
        for (PathComponent::Edge* edge : srcPoint->edges)
        {
            if (edge->destination == dstPoint)
            {
                edgeFound = true;
                break;
            }
        }

        if (edgeFound)
        {
            toRemoveEdge.push_back(srcPoint);
        }
        else
        {
            toAddEdge.push_back(srcPoint);
        }
    }
}

void WayEditSystem::AddEdges(DAVA::PathComponent* path, const DAVA::Vector<DAVA::PathComponent::Waypoint*>& srcPoints, DAVA::PathComponent::Waypoint* nextWaypoint)
{
    DVASSERT(path);
    DVASSERT(nextWaypoint);

    SceneEditor2* sceneEditor = GetSceneEditor();

    for (DAVA::PathComponent::Waypoint* waypoint : srcPoints)
    {
        DAVA::PathComponent::Edge* newEdge = new DAVA::PathComponent::Edge();
        newEdge->destination = nextWaypoint;
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new AddEdgeCommand(sceneEditor, path, waypoint, newEdge)));
    }
}

void WayEditSystem::RemoveEdges(DAVA::PathComponent* path, const DAVA::Vector<DAVA::PathComponent::Waypoint*>& srcPoints, DAVA::PathComponent::Waypoint* nextWaypoint)
{
    using namespace DAVA;
    DVASSERT(path);
    DVASSERT(nextWaypoint);

    SceneEditor2* sceneEditor = GetSceneEditor();
    Vector<std::pair<PathComponent::Waypoint*, PathComponent::Edge*>> edgesToRemove;
    edgesToRemove.reserve(srcPoints.size());

    for (PathComponent::Waypoint* srcPoint : srcPoints)
    {
        for (PathComponent::Edge* edge : srcPoint->edges)
        {
            if (edge->destination == nextWaypoint)
            {
                edgesToRemove.push_back(std::make_pair(srcPoint, edge));
            }
        }
    }

    for (auto& node : edgesToRemove)
    {
        WayEditSystemDetail::AccessibleQueryParams params;
        params.startPoint = path->GetStartWaypoint();
        params.destinationPoint = node.second->destination;
        params.excludeEdge = node.second;
        if (WayEditSystemDetail::IsAccessible(params) == true)
        {
            sceneEditor->Exec(std::make_unique<RemoveEdgeCommand>(sceneEditor, path, node.first, node.second));
        }
    }
}

void WayEditSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandID(CMDID_ENABLE_WAYEDIT))
    {
        DVASSERT(commandNotification.MatchCommandID(CMDID_DISABLE_WAYEDIT) == false);
        EnableWayEdit(commandNotification.redo);
    }
    else if (commandNotification.MatchCommandID(CMDID_DISABLE_WAYEDIT))
    {
        DVASSERT(commandNotification.MatchCommandID(CMDID_ENABLE_WAYEDIT) == false);
        EnableWayEdit(!commandNotification.redo);
    }
}

void WayEditSystem::Draw()
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());

    DAVA::Set<DAVA::PathComponent::Waypoint*> selected;
    if (currentSelection.IsEmpty())
    {
        selected.insert(selectedWaypoints.begin(), selectedWaypoints.end());
    }
    else
    {
        for (auto item : currentSelection.ObjectsOfType<DAVA::Entity>())
        {
            DAVA::WaypointComponent* waypointComponent = GetWaypointComponent(item);
            if (waypointComponent != nullptr)
            {
                selected.insert(waypointComponent->GetWaypoint());
            }
        }
    }

    const DAVA::uint32 count = static_cast<DAVA::uint32>(waypointEntities.size());
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::Entity* e = waypointEntities[i];
        DAVA::Entity* path = e->GetParent();
        DVASSERT(path);

        if (!e->GetVisible() || !path->GetVisible())
        {
            continue;
        }

        DAVA::WaypointComponent* wpComponent = GetWaypointComponent(e);
        DVASSERT(wpComponent);
        DAVA::PathComponent::Waypoint* waypoint = wpComponent->GetWaypoint();

        DAVA::float32 redValue = 0.0f;
        DAVA::float32 greenValue = 0.0f;
        DAVA::float32 blueValue = wpComponent->IsStartingPoint() ? 1.0f : 0.0f;

        if (wpComponent->GetWaypoint() == underCursorPathEntity)
        {
            redValue = 0.6f;
            greenValue = 0.6f;
        }
        else if (selected.count(waypoint))
        {
            redValue = 1.0f;
        }
        else
        {
            greenValue = 1.0f;
        }

        DAVA::AABBox3 localBox = editorScene->collisionSystem->GetUntransformedBoundingBox(e);
        // localBox.IsEmpty() == true, means that "e" was added into system on this frame
        // and collision has not been calculated yet. We will draw this entity on next frame
        if (localBox.IsEmpty() == false)
        {
            editorScene->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(localBox, e->GetWorldTransform(), DAVA::Color(redValue, greenValue, blueValue, 0.3f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
            editorScene->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(localBox, e->GetWorldTransform(), DAVA::Color(redValue, greenValue, blueValue, 1.0f), DAVA::RenderHelper::DRAW_WIRE_DEPTH);
        }
    }
}

void WayEditSystem::EnableWayEdit(bool enable)
{
    ResetSelection();

    isEnabled = enable;
    UpdateSelectionMask();
}

bool WayEditSystem::IsWayEditEnabled() const
{
    return isEnabled;
}

void WayEditSystem::UpdateSelectionMask()
{
    if (isEnabled)
    {
        Selection::SetSelectionComponentMask((DAVA::uint64)1 << DAVA::Component::WAYPOINT_COMPONENT | (DAVA::uint64)1 << DAVA::Component::PATH_COMPONENT);
    }
    else
    {
        Selection::ResetSelectionComponentMask();
    }
}

bool WayEditSystem::HasCustomClonedAddading(DAVA::Entity* entityToClone) const
{
    if (isEnabled == true && GetWaypointComponent(entityToClone))
    {
        return true;
    }

    return false;
}

void WayEditSystem::PerformAdding(DAVA::Entity* sourceEntity, DAVA::Entity* clonedEntity)
{
    using namespace DAVA;

    DVASSERT(isEnabled == true);
    WaypointComponent* sourceComponent = GetWaypointComponent(sourceEntity);
    WaypointComponent* clonedComponent = GetWaypointComponent(clonedEntity);
    DVASSERT(sourceComponent);
    DVASSERT(clonedComponent);

    PathComponent::Waypoint* sourceWayPoint = sourceComponent->GetWaypoint();
    DVASSERT(sourceWayPoint != nullptr);
    sourceWayPoint->position = sourceEntity->GetLocalTransform().GetTranslationVector();

    PathComponent::Waypoint* clonedWayPoint = clonedComponent->GetWaypoint();
    DVASSERT(clonedWayPoint == sourceWayPoint);

    PathComponent* path = sourceComponent->GetPath();

    clonedWayPoint = new PathComponent::Waypoint();
    clonedWayPoint->name = sourceWayPoint->name;
    clonedWayPoint->position = clonedEntity->GetLocalTransform().GetTranslationVector();
    KeyedArchive* propertiesCopy = new KeyedArchive(*sourceWayPoint->GetProperties());
    clonedWayPoint->SetProperties(propertiesCopy);
    SafeRelease(propertiesCopy);

    clonedComponent->Init(path, clonedWayPoint);
    clonedEntity->SetNotRemovable(false);
    sourceEntity->GetParent()->AddNode(clonedEntity);

    UnorderedMap<PathComponent::Edge*, EdgeComponent*> edgeMap;
    for (uint32 i = 0; i < clonedEntity->GetComponentCount(Component::EDGE_COMPONENT); ++i)
    {
        EdgeComponent* edgeComponent = static_cast<EdgeComponent*>(clonedEntity->GetComponent(Component::EDGE_COMPONENT, i));
        edgeMap[edgeComponent->GetEdge()] = edgeComponent;
    }

    // add new waypoint
    SceneEditor2* sceneEditor = GetSceneEditor();
    sceneEditor->Exec(std::make_unique<AddWaypointCommand>(sceneEditor, path, clonedWayPoint));

    // add copy of edges from source point to cloned
    for (PathComponent::Edge* edge : sourceWayPoint->edges)
    {
        PathComponent::Edge* newEdge = new PathComponent::Edge();
        newEdge->destination = edge->destination;
        auto edgeMappingIter = edgeMap.find(edge);
        DVASSERT(edgeMappingIter != edgeMap.end());
        edgeMappingIter->second->Init(path, newEdge);
        sceneEditor->Exec(std::make_unique<AddEdgeCommand>(sceneEditor, path, clonedWayPoint, newEdge));
    }

    // add edge from source point to cloned
    PathComponent::Edge* newEdge = new PathComponent::Edge();
    newEdge->destination = clonedWayPoint;
    sceneEditor->Exec(std::make_unique<AddEdgeCommand>(sceneEditor, path, sourceWayPoint, newEdge));
}

bool WayEditSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();
    bool shiftPressed = keyboard.IsKeyPressed(DAVA::Key::LSHIFT) || keyboard.IsKeyPressed(DAVA::Key::RSHIFT);
    if (isEnabled && shiftPressed)
    {
        return (selectedWaypoints.size() > 0);
    }

    return true;
}

bool WayEditSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    DAVA::Engine* engine = DAVA::Engine::Instance();
    DVASSERT(engine != nullptr);
    const DAVA::EngineContext* engineContext = engine->GetContext();
    if (engineContext->inputSystem == nullptr)
    {
        return true;
    }

    const DAVA::KeyboardDevice& keyboard = engineContext->inputSystem->GetKeyboard();
    bool shiftPressed = keyboard.IsKeyPressed(DAVA::Key::LSHIFT) || keyboard.IsKeyPressed(DAVA::Key::RSHIFT);
    if (isEnabled && shiftPressed)
    {
        // no waypoints selected or no new objects are selected
        // will attempt to create new waypoint in input handler
        if (newSelection.IsEmpty())
            return true;

        // do not allow multiselection here
        if (newSelection.GetSize() > 1)
        {
            return false;
        }

        // only allow to select waypoints in this mode
        auto entity = newSelection.GetFirst().AsEntity();
        if ((entity == nullptr) || (GetWaypointComponent(entity) == nullptr))
        {
            return false;
        }

        // only allow to select waypoints withing same path
        if (entity->GetParent() != GetSceneEditor()->pathSystem->GetCurrrentPath())
        {
            return false;
        }
    }

    return true;
}

SceneEditor2* WayEditSystem::GetSceneEditor() const
{
    return static_cast<SceneEditor2*>(GetScene());
}
