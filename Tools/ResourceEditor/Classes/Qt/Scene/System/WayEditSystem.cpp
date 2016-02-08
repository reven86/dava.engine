/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include <QApplication>
#include "WayEditSystem.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Settings/SettingsManager.h"
#include "Scene/System/PathSystem.h"
#include "Scene/SceneEditor2.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/AddComponentCommand.h"
#include "Commands2/RemoveComponentCommand.h"
#include "Utils/Utils.h"

WayEditSystem::WayEditSystem(DAVA::Scene* scene, SceneSelectionSystem* _selectionSystem, SceneCollisionSystem* _collisionSystem)
    : DAVA::SceneSystem(scene)
    , isEnabled(false)
    , selectionSystem(_selectionSystem)
    , collisionSystem(_collisionSystem)
    , underCursorPathEntity(nullptr)
{
    sceneEditor = static_cast<SceneEditor2*>(GetScene());
}

WayEditSystem::~WayEditSystem()
{
    waypointEntities.clear();
}

void WayEditSystem::AddEntity(DAVA::Entity* newWaypoint)
{
    waypointEntities.push_back(newWaypoint);

    if (newWaypoint->GetNotRemovable())
    {
        mapStartPoints[newWaypoint->GetParent()] = newWaypoint;
    }
}
void WayEditSystem::RemoveEntity(DAVA::Entity* removedPoint)
{
    DAVA::FindAndRemoveExchangingWithLast(waypointEntities, removedPoint);

    if (removedPoint->GetNotRemovable()) // is a start point, remove it from the map of start points
    {
        for (auto iter = mapStartPoints.begin(); iter != mapStartPoints.end(); ++iter)
        {
            if (iter->second == removedPoint)
            {
                mapStartPoints.erase(iter);
                break;
            }
        }
    }
}

void WayEditSystem::WillRemove(DAVA::Entity* removedPoint)
{
    if (IsWayEditEnabled() && GetWaypointComponent(removedPoint))
    {
        startPointForRemove = mapStartPoints[removedPoint->GetParent()];
        DVASSERT(startPointForRemove);
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
            sceneEditor->Exec(new RemoveComponentCommand(waypoint, edge));
            srcPoints.push_back(waypoint);
        }
    }
    // get points aimed by removed point, remove edges
    DAVA::List<DAVA::Entity*> breachPoints;
    DAVA::Entity* dest;
    uint32 count = removedPoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
    for (uint32 i = 0; i < count; ++i)
    {
        edge = static_cast<EdgeComponent*>(removedPoint->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
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

            sceneEditor->Exec(new AddComponentCommand(srcPoint, newEdge));
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
        sceneEditor->Exec(new RemoveComponentCommand(srcWaypoint, edgeComponent));
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

    uint32 count = startPoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
    for (uint32 i = 0; i < count; ++i)
    {
        DAVA::EdgeComponent* edge = static_cast<EdgeComponent*>(startPoint->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
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
    if (isEnabled)
    {
        ProcessSelection();
    }
}

void WayEditSystem::ResetSelection()
{
    selectedWaypoints.Clear();
    prevSelectedWaypoints.Clear();

    underCursorPathEntity = NULL;
}

void WayEditSystem::ProcessSelection()
{
    const EntityGroup& selection = selectionSystem->GetSelection();
    if (currentSelection != selection)
    {
        currentSelection = selection;
        prevSelectedWaypoints = selectedWaypoints;

        selectedWaypoints.Clear();

        for (const auto& item : currentSelection.GetContent())
        {
            Entity* entity = item.first;
            if (GetWaypointComponent(entity) && GetPathComponent(entity->GetParent()))
            {
                selectedWaypoints.Add(entity, selectionSystem->GetSelectionAABox(entity));
            }
        }
    }
}

void WayEditSystem::Input(DAVA::UIEvent* event)
{
    if (isEnabled)
    {
        if ((DAVA::UIEvent::MouseButton::LEFT == event->mouseButton) && (DAVA::UIEvent::Phase::MOVE == event->phase))
        {
            underCursorPathEntity = nullptr;
            const EntityGroup::EntityVector& collObjects = collisionSystem->ObjectsRayTestFromCamera();
            if (!collObjects.empty())
            {
                DAVA::Entity* underEntity = collObjects.front().first;
                if (GetWaypointComponent(underEntity) && GetPathComponent(underEntity->GetParent()))
                {
                    underCursorPathEntity = underEntity;
                }
            }
        }

        if ((DAVA::UIEvent::MouseButton::LEFT == event->mouseButton) && (DAVA::UIEvent::Phase::BEGAN == event->phase))
        {
            inCloneState = sceneEditor->modifSystem->InCloneState();
        }

        if ((DAVA::UIEvent::Phase::ENDED == event->phase) && (DAVA::UIEvent::MouseButton::LEFT == event->mouseButton))
        {
            bool cloneJustDone = false;
            if (inCloneState && !sceneEditor->modifSystem->InCloneState())
            {
                inCloneState = false;
                cloneJustDone = true;
            }

            int curKeyModifiers = QApplication::keyboardModifiers();
            if (0 == (curKeyModifiers & Qt::ShiftModifier))
            { //we need use shift key to add waypoint or edge
                return;
            }

            Entity* currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
            if (!currentWayParent)
            { // we need have entity with path component
                return;
            }

            ProcessSelection();

            if (!selectedWaypoints.IsEmpty())
            {
                if (selectedWaypoints.Size() == 1 && !cloneJustDone)
                {
                    Entity* nextWaypoint = selectedWaypoints.GetFirstEntity();

                    EntityGroup entitiesToAddEdge;
                    EntityGroup entitiesToRemoveEdge;
                    DefineAddOrRemoveEdges(prevSelectedWaypoints, nextWaypoint, entitiesToAddEdge, entitiesToRemoveEdge);
                    const size_t countToAdd = entitiesToAddEdge.Size();
                    const size_t countToRemove = entitiesToRemoveEdge.Size();

                    if ((countToAdd + countToRemove) > 0)
                    {
                        sceneEditor->BeginBatch(DAVA::Format("Add/remove edges pointed on entity %s", nextWaypoint->GetName().c_str()));

                        AddEdges(entitiesToAddEdge, nextWaypoint);
                        RemoveEdges(entitiesToRemoveEdge, nextWaypoint);

                        sceneEditor->EndBatch();
                    }
                }
            }
            else
            {
                DAVA::Vector3 lanscapeIntersectionPos;
                bool lanscapeIntersected = collisionSystem->LandRayTestFromCamera(lanscapeIntersectionPos);

                // add new waypoint on the landscape
                if (lanscapeIntersected)
                {
                    EntityGroup validPrevPoints;
                    FilterPrevSelection(currentWayParent, validPrevPoints);
                    if (validPrevPoints.IsEmpty())
                    {
                        if (currentWayParent->CountChildEntitiesWithComponent(DAVA::Component::WAYPOINT_COMPONENT) > 0)
                        {
                            // current path has waypoints but none of them was selected. Point adding is denied
                            return;
                        }
                    }

                    sceneEditor->BeginBatch("Add Waypoint");

                    Entity* newWaypoint = CreateWayPoint(currentWayParent, lanscapeIntersectionPos);
                    sceneEditor->Exec(new EntityAddCommand(newWaypoint, currentWayParent));

                    if (!validPrevPoints.IsEmpty())
                    {
                        AddEdges(validPrevPoints, newWaypoint);
                    }

                    sceneEditor->EndBatch();

                    newWaypoint->Release();
                }
            }
        }
    }
}

void WayEditSystem::FilterPrevSelection(DAVA::Entity* parentEntity, EntityGroup& ret)
{
    for (const auto& item : prevSelectedWaypoints.GetContent())
    {
        if (parentEntity == item.first->GetParent())
        {
            ret.Add(item.first, selectionSystem->GetSelectionAABox(item.first));
        }
    }
}

void WayEditSystem::DefineAddOrRemoveEdges(const EntityGroup& srcPoints, DAVA::Entity* dstPoint, EntityGroup& toAddEdge, EntityGroup& toRemoveEdge)
{
    for (const auto& item : srcPoints.GetContent())
    {
        Entity* srcPoint = item.first;
        if (dstPoint->GetParent() != srcPoint->GetParent())
        {
            //we don't allow connect different pathes
            continue;
        }

        if (FindEdgeComponent(srcPoint, dstPoint))
        {
            toRemoveEdge.Add(srcPoint, selectionSystem->GetSelectionAABox(srcPoint));
        }
        else
        {
            toAddEdge.Add(srcPoint, selectionSystem->GetSelectionAABox(srcPoint));
        }
    }
}

void WayEditSystem::AddEdges(const EntityGroup& group, DAVA::Entity* nextEntity)
{
    DVASSERT(nextEntity);

    for (const auto& item : group.GetContent())
    {
        DAVA::EdgeComponent* edge = new DAVA::EdgeComponent();
        edge->SetNextEntity(nextEntity);
        sceneEditor->Exec(new AddComponentCommand(item.first, edge));
    }
}

void WayEditSystem::RemoveEdges(const EntityGroup& group, DAVA::Entity* nextEntity)
{
    for (const auto& item : group.GetContent())
    {
        EdgeComponent* edgeToNextEntity = FindEdgeComponent(item.first, nextEntity);
        DVASSERT(edgeToNextEntity);
        RemoveEdge(item.first, edgeToNextEntity);
    }
}

DAVA::Entity* WayEditSystem::CreateWayPoint(DAVA::Entity* parent, DAVA::Vector3 pos)
{
    DAVA::PathComponent* pc = DAVA::GetPathComponent(parent);
    DVASSERT(pc);

    DAVA::Entity* waypoint = new DAVA::Entity();

    const int32 childrenCount = parent->CountChildEntitiesWithComponent(DAVA::Component::WAYPOINT_COMPONENT);
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

void WayEditSystem::ProcessCommand(const Command2* command, bool redo)
{
    const int commandId = command->GetId();
    if (commandId == CMDID_ENABLE_WAYEDIT)
    {
        EnableWayEdit(redo);
    }
    else if (commandId == CMDID_DISABLE_WAYEDIT)
    {
        EnableWayEdit(!redo);
    }
}

void WayEditSystem::Draw()
{
    const EntityGroup& selectionGroup = (currentSelection.IsEmpty()) ? prevSelectedWaypoints : currentSelection;

    const uint32 count = waypointEntities.size();
    for (uint32 i = 0; i < count; ++i)
    {
        Entity* e = waypointEntities[i];
        Entity* path = e->GetParent();
        DVASSERT(path);

        if (!e->GetVisible() || !path->GetVisible())
        {
            continue;
        }

        DAVA::WaypointComponent* wpComponent = GetWaypointComponent(e);
        DVASSERT(wpComponent);

        AABBox3 worldBox = selectionSystem->GetSelectionAABox(e);

        float32 redValue = 0.0f;
        float32 greenValue = 0.0f;
        float32 blueValue = wpComponent->IsStarting() ? 1.0f : 0.0f;

        if (e == underCursorPathEntity)
        {
            redValue = 0.6f;
            greenValue = 0.6f;
        }
        else if (selectionGroup.ContainsEntity(e))
        {
            redValue = 1.0f;
        }
        else
        {
            greenValue = 1.0f;
        }

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(worldBox, e->GetWorldTransform(), DAVA::Color(redValue, greenValue, blueValue, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(worldBox, e->GetWorldTransform(), DAVA::Color(redValue, greenValue, blueValue, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
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

        sceneEditor->Exec(new AddComponentCommand(originalEntity, edge));
    }
}
