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


WayEditSystem::WayEditSystem(DAVA::Scene * scene, SceneSelectionSystem *_selectionSystem, SceneCollisionSystem *_collisionSystem)
    : DAVA::SceneSystem(scene)
    , isEnabled(false)
    , selectionSystem(_selectionSystem)
    , collisionSystem(_collisionSystem)
    , underCursorPathEntity(nullptr)
{
    sceneEditor = static_cast<SceneEditor2 *>(GetScene());
}

WayEditSystem::~WayEditSystem()
{
    waypointEntities.clear();
}

void WayEditSystem::AddEntity(DAVA::Entity * newWaypoint)
{
    waypointEntities.push_back(newWaypoint);

    if (newWaypoint->GetNotRemovable())
    {
        mapStartPoints[newWaypoint->GetParent()] = newWaypoint;
    }
}
void WayEditSystem::RemoveEntity(DAVA::Entity * removedPoint)
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

void WayEditSystem::WillRemove(DAVA::Entity *removedPoint)
{
    if (IsWayEditEnabled() && GetWaypointComponent(removedPoint))
    {
        startPointForRemove = mapStartPoints[removedPoint->GetParent()];
        DVASSERT(startPointForRemove);
    }
}

void WayEditSystem::DidRemoved(DAVA::Entity *removedPoint)
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
        if (IsAccessible(startPointForRemove, *breachPoint, removedPoint, nullptr/*no excluding edge*/, passedPoints))
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

            DAVA::EdgeComponent *newEdge = new DAVA::EdgeComponent();
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
    if (IsAccessible(startPoint, breachPoint, nullptr/*no excluding point*/, edgeComponent, passedPoints))
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
            return  true;
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
    const EntityGroup selection = selectionSystem->GetSelection();
    if (currentSelection != selection)
    {
        currentSelection = selection;
        prevSelectedWaypoints = selectedWaypoints;

        selectedWaypoints.Clear();

        const size_t count = currentSelection.Size();
        for(size_t i = 0; i < count; ++i)
        {
            Entity * entity = currentSelection.GetEntity(i);
            if(GetWaypointComponent(entity) && GetPathComponent(entity->GetParent()))
            {
                selectedWaypoints.Add(entity);
            }
        }
    }
}

void WayEditSystem::Input(DAVA::UIEvent *event)
{
    if (isEnabled)
    {
        if((DAVA::UIEvent::BUTTON_1 == event->tid) && (DAVA::UIEvent::PHASE_MOVE == event->phase))
        {
            underCursorPathEntity = nullptr;
            const EntityGroup* collObjects = collisionSystem->ObjectsRayTestFromCamera();
            if (NULL != collObjects && collObjects->Size() > 0)
            {
                DAVA::Entity *underEntity = collObjects->GetEntity(0);
                if (GetWaypointComponent(underEntity) && GetPathComponent(underEntity->GetParent()))
                {
                    underCursorPathEntity = underEntity;
                }
            }
        }

        if ((DAVA::UIEvent::BUTTON_1 == event->tid) && (DAVA::UIEvent::PHASE_BEGAN == event->phase))
        {
            inCloneState = sceneEditor->modifSystem->InCloneState();
        }
        
        if ((DAVA::UIEvent::PHASE_ENDED == event->phase) && (DAVA::UIEvent::BUTTON_1 == event->tid))
        {
            bool cloneJustDone = false;
            if (inCloneState && !sceneEditor->modifSystem->InCloneState())
            {
                inCloneState = false;
                cloneJustDone = true;
            }


            int curKeyModifiers = QApplication::keyboardModifiers();
            if(0 == (curKeyModifiers & Qt::ShiftModifier))
            {   //we need use shift key to add waypoint or edge
                return;
            }

            Entity * currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
            if(!currentWayParent)
            {   // we need have entity with path component
                return;
            }

            
            ProcessSelection();

            if(selectedWaypoints.Size() != 0)
            {
                if(selectedWaypoints.Size() == 1 && !cloneJustDone)
                {
                    Entity *nextWaypoint = selectedWaypoints.GetEntity(0);

                    EntityGroup entitiesToAddEdge;
                    EntityGroup entitiesToRemoveEdge;
                    DefineAddOrRemoveEdges(prevSelectedWaypoints, nextWaypoint, entitiesToAddEdge, entitiesToRemoveEdge);
                    const size_t countToAdd = entitiesToAddEdge.Size();
                    const size_t countToRemove = entitiesToRemoveEdge.Size();

                    if((countToAdd + countToRemove) > 0)
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
                    EntityGroup validPrevPoints = FilterPrevSelection(currentWayParent);
                    if (!validPrevPoints.Size())
                    {
                        if (currentWayParent->CountChildEntitiesWithComponent(DAVA::Component::WAYPOINT_COMPONENT) > 0)
                        {
                            // current path has waypoints but none of them was selected. Point adding is denied
                            return;
                        }
                    }

                    sceneEditor->BeginBatch("Add Waypoint");

                    Entity *newWaypoint = CreateWayPoint(currentWayParent, lanscapeIntersectionPos);
                    sceneEditor->Exec(new EntityAddCommand(newWaypoint, currentWayParent));
                    
                    if(validPrevPoints.Size())
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

EntityGroup WayEditSystem::FilterPrevSelection(DAVA::Entity *parentEntity)
{
    EntityGroup ret;

    const size_t count = prevSelectedWaypoints.Size();
    for (size_t i = 0; i < count; ++i)
    {
        Entity * entity = prevSelectedWaypoints.GetEntity(i);
        if (parentEntity == entity->GetParent())
        {
            ret.Add(entity);
        }
    }

    return ret;
}

void WayEditSystem::DefineAddOrRemoveEdges(const EntityGroup& srcPoints, DAVA::Entity* dstPoint, EntityGroup& toAddEdge, EntityGroup& toRemoveEdge)
{
    const size_t count = srcPoints.Size();
    for(size_t i = 0; i < count; ++i)
    {
        Entity * srcPoint = srcPoints.GetEntity(i);
        if(dstPoint->GetParent() != srcPoint->GetParent())
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


void WayEditSystem::AddEdges(const EntityGroup & group, DAVA::Entity *nextEntity)
{
    DVASSERT(nextEntity);
    
    const size_t count = group.Size();
    for(size_t i = 0; i < count; ++i)
    {
        Entity * entity = group.GetEntity(i);

        DAVA::EdgeComponent *edge = new DAVA::EdgeComponent();
        edge->SetNextEntity(nextEntity);
        
        sceneEditor->Exec(new AddComponentCommand(entity, edge));
    }
}

void WayEditSystem::RemoveEdges(const EntityGroup & group, DAVA::Entity *nextEntity)
{
    const size_t count = group.Size();
    for (size_t i = 0; i < count; ++i)
    {
        Entity * srcEntity = group.GetEntity(i);
        EdgeComponent* edgeToNextEntity = FindEdgeComponent(srcEntity, nextEntity);
        DVASSERT(edgeToNextEntity);
        RemoveEdge(srcEntity, edgeToNextEntity);
    }
}

DAVA::Entity* WayEditSystem::CreateWayPoint(DAVA::Entity *parent, DAVA::Vector3 pos)
{
    DAVA::PathComponent *pc = DAVA::GetPathComponent(parent);
    DVASSERT(pc);

    DAVA::Entity *waypoint = new DAVA::Entity();

    const int32 childrenCount = parent->CountChildEntitiesWithComponent(DAVA::Component::WAYPOINT_COMPONENT);
    waypoint->SetName(DAVA::FastName(DAVA::Format("Waypoint_%d", childrenCount)));

    DAVA::WaypointComponent *wc = new DAVA::WaypointComponent();
    wc->SetPathName(pc->GetName());
    if (childrenCount==0)
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

void WayEditSystem::ProcessCommand(const Command2 *command, bool redo)
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
    const EntityGroup & selectionGroup = (currentSelection.Size()) ? currentSelection : prevSelectedWaypoints;

    const uint32 count = waypointEntities.size();
    for(uint32 i = 0; i < count; ++i)
    {
        Entity *e = waypointEntities[i];
        Entity *path = e->GetParent();
        DVASSERT(path);
        
        if(!e->GetVisible() || !path->GetVisible())
        {
            continue;
        }
        
        DAVA::WaypointComponent* wpComponent = GetWaypointComponent(e);
        DVASSERT(wpComponent);
        
        AABBox3 worldBox = selectionSystem->GetSelectionAABox(e);

        float32 redValue = 0.0f;
        float32 greenValue = 0.0f;
        float32 blueValue = wpComponent->IsStarting() ? 1.0f : 0.0f;
        
        if(e == underCursorPathEntity)
        {
            redValue = 0.6f;
            greenValue = 0.6f;
        }
        else if(selectionGroup.ContainsEntity(e))
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
    if(isEnabled)
    {
        selectionSystem->SetSelectionComponentMask((DAVA::uint64)1 << DAVA::Component::WAYPOINT_COMPONENT | (DAVA::uint64)1 << DAVA::Component::PATH_COMPONENT);
    }
    else
    {
        selectionSystem->ResetSelectionComponentMask();
    }
}

void WayEditSystem::WillClone(DAVA::Entity *originalEntity)
{
}

void WayEditSystem::DidCloned(DAVA::Entity *originalEntity, DAVA::Entity *newEntity)
{
    if (isEnabled && GetWaypointComponent(originalEntity) != nullptr)
    {
        DAVA::EdgeComponent *edge = new DAVA::EdgeComponent();
        edge->SetNextEntity(newEntity);

        sceneEditor->Exec(new AddComponentCommand(originalEntity, edge));
    }
}
