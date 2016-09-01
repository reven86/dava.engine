#include <QApplication>
#include "WayEditSystem.h"
#include "Math/AABBox3.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Settings/SettingsManager.h"
#include "Scene/System/PathSystem.h"
#include "Scene/SceneEditor2.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/AddComponentCommand.h"
#include "Commands2/RemoveComponentCommand.h"
#include "Commands2/Base/RECommandNotificationObject.h"
#include "Math/AABBox3.h"
#include "Utils/Utils.h"

#include "Debug/DVAssert.h"

WayEditSystem::WayEditSystem(DAVA::Scene* scene, SceneSelectionSystem* _selectionSystem, SceneCollisionSystem* _collisionSystem)
    : DAVA::SceneSystem(scene)
    , sceneEditor(static_cast<SceneEditor2*>(scene))
    , selectionSystem(_selectionSystem)
    , collisionSystem(_collisionSystem)
{
}

void WayEditSystem::AddEntity(DAVA::Entity* newWaypoint)
{
    waypointEntities.push_back(newWaypoint);

    if (newWaypoint->GetNotRemovable())
    {
        auto parent = newWaypoint->GetParent();

        // allow only one start point
        DVASSERT(mapStartPoints.count(parent) == 0);

        mapStartPoints[parent] = newWaypoint;
    }
}

void WayEditSystem::RemoveEntity(DAVA::Entity* removedPoint)
{
    DAVA::FindAndRemoveExchangingWithLast(waypointEntities, removedPoint);

    if (removedPoint->GetNotRemovable() == false)
        return;

    for (auto iter = mapStartPoints.begin(); iter != mapStartPoints.end(); ++iter)
    {
        if (iter->second == removedPoint)
        {
            mapStartPoints.erase(iter);
            return;
        }
    }

    DVASSERT(0, "Invalid (not tracked) starting waypoint removed");
}

void WayEditSystem::WillRemove(DAVA::Entity* removedPoint)
{
    if (IsWayEditEnabled() && GetWaypointComponent(removedPoint))
    {
        auto i = mapStartPoints.find(removedPoint->GetParent());
        DVASSERT(i != mapStartPoints.end());
        startPointForRemove = i->second;
    }
}

void WayEditSystem::DidRemoved(DAVA::Entity* removedPoint)
{
    if (!IsWayEditEnabled() || !GetWaypointComponent(removedPoint))
    {
        return;
    }

    DAVA::EdgeComponent* edge;

    // get points aiming at removed point, remove edges
    DAVA::List<DAVA::Entity*> srcPoints;
    for (auto waypoint : waypointEntities)
    {
        edge = FindEdgeComponent(waypoint, removedPoint);
        if (edge)
        {
            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new RemoveComponentCommand(waypoint, edge)));
            srcPoints.push_back(waypoint);
        }
    }
    // get points aimed by removed point, remove edges
    DAVA::List<DAVA::Entity*> breachPoints;
    DAVA::Entity* dest;
    DAVA::uint32 count = removedPoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        edge = static_cast<DAVA::EdgeComponent*>(removedPoint->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
        DVASSERT(edge);

        dest = edge->GetNextEntity();
        if (dest)
        {
            breachPoints.push_back(dest);
        }
    }

    // detect really breached points
    for (auto breachPoint = breachPoints.begin(); breachPoint != breachPoints.end();)
    {
        DAVA::Set<DAVA::Entity*> passedPoints;
        if (IsAccessible(startPointForRemove, *breachPoint, removedPoint, nullptr /*no excluding edge*/, passedPoints))
        {
            auto delPoint = breachPoint++;
            breachPoints.erase(delPoint);
        }
        else
            ++breachPoint;
    }
    startPointForRemove = nullptr;

    // link source points and breached points
    for (auto breachPoint : breachPoints)
    {
        for (auto srcPoint : srcPoints)
        {
            if (srcPoint == breachPoint)
                continue;

            DAVA::EdgeComponent* newEdge = new DAVA::EdgeComponent();
            newEdge->SetNextEntity(breachPoint);

            sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(srcPoint, newEdge)));
        }
    }
}

void WayEditSystem::RemoveEdge(DAVA::Entity* srcWaypoint, DAVA::EdgeComponent* edgeComponent)
{
    DAVA::Entity* breachPoint = edgeComponent->GetNextEntity();
    DAVA::Entity* startPoint = mapStartPoints[breachPoint->GetParent()];
    DVASSERT(startPoint);

    DAVA::Set<DAVA::Entity*> passedPoints;
    if (IsAccessible(startPoint, breachPoint, nullptr /*no excluding point*/, edgeComponent, passedPoints))
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new RemoveComponentCommand(srcWaypoint, edgeComponent)));
    }
}

bool WayEditSystem::IsAccessible(DAVA::Entity* startPoint,
                                 DAVA::Entity* breachPoint,
                                 DAVA::Entity* excludingPoint,
                                 DAVA::EdgeComponent* excludingEdge,
                                 DAVA::Set<DAVA::Entity*>& passedPoints) const
{
    if (startPoint == breachPoint)
        return true;

    DAVA::uint32 count = startPoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::EdgeComponent* edge = static_cast<DAVA::EdgeComponent*>(startPoint->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
        DVASSERT(edge);

        if (edge == excludingEdge)
        {
            continue;
        }

        DAVA::Entity* nextPoint = edge->GetNextEntity();
        DVASSERT(nextPoint);

        if (nextPoint == excludingPoint)
        {
            continue;
        }

        auto insertRes = passedPoints.insert(nextPoint);
        if (!insertRes.second)
        {
            // dest is already inserted. skip it to prevent loop
            continue;
        }

        if (IsAccessible(nextPoint, breachPoint, excludingPoint, excludingEdge, passedPoints))
        {
            return true;
        }
    }

    return false;
}

void WayEditSystem::Process(DAVA::float32 timeElapsed)
{
}

void WayEditSystem::ResetSelection()
{
    selectedWaypoints.Clear();
    prevSelectedWaypoints.Clear();

    underCursorPathEntity = NULL;
}

void WayEditSystem::ProcessSelection(const SelectableGroup& selection)
{
    prevSelectedWaypoints = selectedWaypoints;
    selectedWaypoints.Clear();

    if (currentSelection != selection)
    {
        currentSelection = selection;

        for (auto entity : currentSelection.ObjectsOfType<DAVA::Entity>())
        {
            if (GetWaypointComponent(entity) && GetPathComponent(entity->GetParent()))
            {
                selectedWaypoints.Add(entity);
            }
        }
    }
}

void WayEditSystem::Input(DAVA::UIEvent* event)
{
    if (isEnabled && (DAVA::UIEvent::MouseButton::LEFT == event->mouseButton))
    {
        if (DAVA::UIEvent::Phase::MOVE == event->phase)
        {
            underCursorPathEntity = nullptr;
            const SelectableGroup::CollectionType& collObjects = collisionSystem->ObjectsRayTestFromCamera();
            if (collObjects.size() == 1)
            {
                DAVA::Entity* firstEntity = collObjects.front().AsEntity();
                if ((firstEntity != nullptr) && GetWaypointComponent(firstEntity) && GetPathComponent(firstEntity->GetParent()))
                {
                    underCursorPathEntity = firstEntity;
                }
            }
        }
        else if (DAVA::UIEvent::Phase::BEGAN == event->phase)
        {
            inCloneState = sceneEditor->modifSystem->InCloneState();
        }
        else if (DAVA::UIEvent::Phase::ENDED == event->phase)
        {
            bool cloneJustDone = false;
            if (inCloneState && !sceneEditor->modifSystem->InCloneState())
            {
                inCloneState = false;
                cloneJustDone = true;
            }

            ProcessSelection(selectionSystem->GetSelection());

            const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();
            bool shiftPressed = keyboard.IsKeyPressed(DAVA::Key::LSHIFT) || keyboard.IsKeyPressed(DAVA::Key::RSHIFT);

            if (!shiftPressed)
            {
                // we need to use shift key to add waypoint or edge
                return;
            }

            DAVA::Entity* currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
            if (currentWayParent == nullptr)
            {
                // we need to have entity with path component
                return;
            }

            if (selectedWaypoints.IsEmpty())
            {
                DAVA::Vector3 lanscapeIntersectionPos;
                bool lanscapeIntersected = collisionSystem->LandRayTestFromCamera(lanscapeIntersectionPos);

                // add new waypoint on the landscape
                if (lanscapeIntersected)
                {
                    SelectableGroup validPrevPoints;
                    FilterPrevSelection(currentWayParent, validPrevPoints);
                    if (validPrevPoints.IsEmpty())
                    {
                        if (currentWayParent->CountChildEntitiesWithComponent(DAVA::Component::WAYPOINT_COMPONENT) > 0)
                        {
                            // current path has waypoints but none of them was selected. Point adding is denied
                            return;
                        }
                    }

                    DAVA::Entity* newWaypoint = CreateWayPoint(currentWayParent, lanscapeIntersectionPos);

                    sceneEditor->selectionSystem->SetLocked(true);
                    sceneEditor->BeginBatch("Add Waypoint", 1 + validPrevPoints.GetSize());
                    sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new EntityAddCommand(newWaypoint, currentWayParent)));
                    if (!validPrevPoints.IsEmpty())
                    {
                        AddEdges(validPrevPoints, newWaypoint);
                    }
                    sceneEditor->EndBatch();

                    selectedWaypoints.Clear();
                    selectedWaypoints.Add(newWaypoint);
                    sceneEditor->selectionSystem->SetLocked(false);
                    newWaypoint->Release();
                }
            }
            else if ((selectedWaypoints.GetSize() == 1) && (cloneJustDone == false))
            {
                DAVA::Entity* nextWaypoint = selectedWaypoints.GetFirst().AsEntity();
                SelectableGroup entitiesToAddEdge;
                SelectableGroup entitiesToRemoveEdge;
                DefineAddOrRemoveEdges(prevSelectedWaypoints, nextWaypoint, entitiesToAddEdge, entitiesToRemoveEdge);
                size_t totalOperations = entitiesToAddEdge.GetSize() + entitiesToRemoveEdge.GetSize();
                if (totalOperations > 0)
                {
                    sceneEditor->BeginBatch(DAVA::Format("Add/remove edges pointed on entity %s", nextWaypoint->GetName().c_str()), totalOperations);
                    AddEdges(entitiesToAddEdge, nextWaypoint);
                    RemoveEdges(entitiesToRemoveEdge, nextWaypoint);
                    sceneEditor->EndBatch();
                }
            }
        }
    }
}

void WayEditSystem::FilterPrevSelection(DAVA::Entity* parentEntity, SelectableGroup& ret)
{
    for (auto entity : prevSelectedWaypoints.ObjectsOfType<DAVA::Entity>())
    {
        if (parentEntity == entity->GetParent())
        {
            ret.Add(entity);
        }
    }
}

void WayEditSystem::DefineAddOrRemoveEdges(const SelectableGroup& srcPoints, DAVA::Entity* dstPoint, SelectableGroup& toAddEdge, SelectableGroup& toRemoveEdge)
{
    for (auto srcPoint : prevSelectedWaypoints.ObjectsOfType<DAVA::Entity>())
    {
        if (dstPoint->GetParent() != srcPoint->GetParent())
        {
            //we don't allow connect different pathes
            continue;
        }

        if (FindEdgeComponent(srcPoint, dstPoint))
        {
            toRemoveEdge.Add(srcPoint);
        }
        else
        {
            toAddEdge.Add(srcPoint);
        }
    }
}

void WayEditSystem::AddEdges(const SelectableGroup& group, DAVA::Entity* nextEntity)
{
    DVASSERT(nextEntity);

    for (auto entity : group.ObjectsOfType<DAVA::Entity>())
    {
        DAVA::EdgeComponent* edge = new DAVA::EdgeComponent();
        edge->SetNextEntity(nextEntity);
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(entity, edge)));
    }
}

void WayEditSystem::RemoveEdges(const SelectableGroup& group, DAVA::Entity* nextEntity)
{
    for (auto entity : group.ObjectsOfType<DAVA::Entity>())
    {
        DAVA::EdgeComponent* edgeToNextEntity = FindEdgeComponent(entity, nextEntity);
        DVASSERT(edgeToNextEntity);
        RemoveEdge(entity, edgeToNextEntity);
    }
}

DAVA::Entity* WayEditSystem::CreateWayPoint(DAVA::Entity* parent, DAVA::Vector3 pos)
{
    DAVA::PathComponent* pc = DAVA::GetPathComponent(parent);
    DVASSERT(pc);

    DAVA::Entity* waypoint = new DAVA::Entity();

    const DAVA::int32 childrenCount = parent->CountChildEntitiesWithComponent(DAVA::Component::WAYPOINT_COMPONENT);
    waypoint->SetName(DAVA::FastName(DAVA::Format("Waypoint_%d", childrenCount)));

    DAVA::WaypointComponent* wc = new DAVA::WaypointComponent();
    wc->SetPathName(pc->GetName());
    if (childrenCount == 0)
    {
        waypoint->SetNotRemovable(true);
        wc->SetStarting(true);
    }
    waypoint->AddComponent(wc);

    DAVA::Matrix4 pm = parent->GetWorldTransform();
    pm.Inverse();

    DAVA::Matrix4 m;
    m.SetTranslationVector(pos);
    waypoint->SetLocalTransform(m * pm);

    return waypoint;
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
    const SelectableGroup& selectionGroup = (currentSelection.IsEmpty()) ? selectedWaypoints : currentSelection;

    const DAVA::uint32 count = waypointEntities.size();
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

        DAVA::float32 redValue = 0.0f;
        DAVA::float32 greenValue = 0.0f;
        DAVA::float32 blueValue = wpComponent->IsStarting() ? 1.0f : 0.0f;

        if (e == underCursorPathEntity)
        {
            redValue = 0.6f;
            greenValue = 0.6f;
        }
        else if (selectionGroup.ContainsObject(e))
        {
            redValue = 1.0f;
        }
        else
        {
            greenValue = 1.0f;
        }

        DAVA::AABBox3 localBox = selectionSystem->GetUntransformedBoundingBox(e);
        DVASSERT(!localBox.IsEmpty());
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(localBox, e->GetWorldTransform(), DAVA::Color(redValue, greenValue, blueValue, 0.3f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(localBox, e->GetWorldTransform(), DAVA::Color(redValue, greenValue, blueValue, 1.0f), DAVA::RenderHelper::DRAW_WIRE_DEPTH);
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
        selectionSystem->SetSelectionComponentMask((DAVA::uint64)1 << DAVA::Component::WAYPOINT_COMPONENT | (DAVA::uint64)1 << DAVA::Component::PATH_COMPONENT);
    }
    else
    {
        selectionSystem->ResetSelectionComponentMask();
    }
}

void WayEditSystem::WillClone(DAVA::Entity* originalEntity)
{
}

void WayEditSystem::DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity)
{
    if (isEnabled && GetWaypointComponent(originalEntity) != nullptr)
    {
        DAVA::EdgeComponent* edge = new DAVA::EdgeComponent();
        edge->SetNextEntity(newEntity);

        if (newEntity->GetNotRemovable())
        {
            // prevent creating second "start" point
            newEntity->SetNotRemovable(false);
        }

        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new AddComponentCommand(originalEntity, edge)));
    }
}

bool WayEditSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();
    bool shiftPressed = keyboard.IsKeyPressed(DAVA::Key::LSHIFT) || keyboard.IsKeyPressed(DAVA::Key::RSHIFT);
    if (isEnabled && shiftPressed)
    {
        return (selectedWaypoints.GetSize() > 0);
    }

    return true;
}

bool WayEditSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();
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
        if (entity->GetParent() != sceneEditor->pathSystem->GetCurrrentPath())
        {
            return false;
        }
    }

    return true;
}
