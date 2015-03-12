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
, underCursorPathEntity(NULL)
{
    wayDrawState = DAVA::RenderManager::Instance()->Subclass3DRenderState(
        DAVA::RenderStateData::STATE_COLORMASK_ALL |
        DAVA::RenderStateData::STATE_DEPTH_TEST);

    sceneEditor = static_cast<SceneEditor2 *>(GetScene());
}

WayEditSystem::~WayEditSystem()
{
    waypointEntities.clear();
}

void WayEditSystem::AddEntity(DAVA::Entity * newWaypoint)
{
    waypointEntities.push_back(newWaypoint);


    if (sceneEditor->modifSystem->InCloneDoneState())
    {
        ProcessSelection();

        EntityGroup entitiesToAddEdge;
        EntityGroup entitiesToRemoveEdge;
        DefineAddOrRemoveEdges(prevSelectedWaypoints, newWaypoint, entitiesToAddEdge, entitiesToRemoveEdge);
        const size_t countToAdd = entitiesToAddEdge.Size();
        const size_t countToRemove = entitiesToRemoveEdge.Size();

        if ((countToAdd + countToRemove) > 0)
        {
            AddEdges(entitiesToAddEdge, newWaypoint);
            RemoveEdges(entitiesToRemoveEdge, newWaypoint);
        }
    }
}
void WayEditSystem::RemoveEntity(DAVA::Entity * removedPoint)
{
    DAVA::FindAndRemoveExchangingWithLast(waypointEntities, removedPoint);
}

void WayEditSystem::RemovePointsGroup(const EntityGroup &entityGroup)
{
    sceneEditor->BeginBatch("Remove entities");

    size_t size = entityGroup.Size();
    for (size_t i = 0; i < size; ++i)
    {
        DAVA::Entity* entity = entityGroup.GetEntity(i);
        if (entity->GetNotRemovable() == false)
        {
            RemoveWayPoint(entity);
        }
    }

    sceneEditor->EndBatch();
}

void WayEditSystem::RemoveWayPoint(DAVA::Entity* removedPoint)
{
    sceneEditor->Exec(new EntityRemoveCommand(removedPoint));

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
        if(dest)
        {
            breachPoints.push_back(dest);
        }

    }

    // detect really breached points
    for (auto breachPoint = breachPoints.begin(); breachPoint != breachPoints.end();)
    {
        auto HasEdgeToBreachPoint = [&](DAVA::Entity* src)
        {
            return (FindEdgeComponent(src, *breachPoint) != nullptr);
        };

        if (any_of(waypointEntities.begin(), waypointEntities.end(), HasEdgeToBreachPoint))
        {
            auto delPoint = breachPoint++;
            breachPoints.erase(delPoint);
        }
        else
            ++breachPoint;
    }

    // link source points and breached points
    for (auto breachPoint : breachPoints)
    {
        for (auto srcPoint : srcPoints)
        {
            if (srcPoint == breachPoint)
                continue;

            DAVA::EdgeComponent *edge = new DAVA::EdgeComponent();
            edge->SetNextEntity(breachPoint);

            sceneEditor->Exec(new AddComponentCommand(srcPoint, edge));
        }
    }
}

void WayEditSystem::RemoveEdge(DAVA::Entity* srcWaypoint, DAVA::EdgeComponent* edgeComponent)
{
    DAVA::Entity* breachPoint = edgeComponent->GetNextEntity();

    auto HasEdgeToBreachPoint = [&](DAVA::Entity* src)
    {
        return (FindEdgeComponent(src, breachPoint) != nullptr);
    };

    if (count_if(waypointEntities.begin(), waypointEntities.end(), HasEdgeToBreachPoint) > 1)
    {
        sceneEditor->Exec(new RemoveComponentCommand(srcWaypoint, edgeComponent));
    }
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
    if(currentSelection != selection)
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
            underCursorPathEntity = NULL;
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
                        if (currentWayParent->GetComponentCount(DAVA::Component::WAYPOINT_COMPONENT) > 0)
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
                    
                    selectionSystem->SetSelection(newWaypoint);
                    
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

    const int32 childrenCount = parent->GetChildrenCount();
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
    //Enable system without running commands
    const int commandId = command->GetId();
    if(CMDID_COLLAPSE_PATH == commandId)
    {
        isEnabled = !redo;
		UpdateSelectionMask();
    }
    else if(CMDID_EXPAND_PATH == commandId)
    {
        isEnabled = redo;
		UpdateSelectionMask();
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

        RenderManager::SetDynamicParam(PARAM_WORLD, &e->GetWorldTransform(), (pointer_size)&e->GetWorldTransform());
        
        AABBox3 worldBox = selectionSystem->GetSelectionAABox(e);
        
        float32 redValue = 0.0f;
        float32 greenValue = 0.0f;
        float32 blueValue = wpComponent->IsStarting() ? 1.0f : 0.0f;
        
        if(e == underCursorPathEntity)
        {
            redValue = 0.6f;
            greenValue = 0.6f;
        }
        else if(selectionGroup.HasEntity(e))
        {
            redValue = 1.0f;
        }
        else
        {
            greenValue = 1.0f;
        }
        
        DAVA::RenderManager::Instance()->SetColor(DAVA::Color(redValue, greenValue, blueValue, 0.3f));
        DAVA::RenderHelper::Instance()->FillBox(worldBox, wayDrawState);
        DAVA::RenderManager::Instance()->SetColor(DAVA::Color(redValue, greenValue, blueValue, 1.0f));
        DAVA::RenderHelper::Instance()->DrawBox(worldBox, 1.0f, wayDrawState);
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


