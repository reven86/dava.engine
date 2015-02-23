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
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
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
}

WayEditSystem::~WayEditSystem()
{
    waypointEntities.clear();
}

void WayEditSystem::AddEntity(DAVA::Entity * entity)
{
    waypointEntities.push_back(entity);
}
void WayEditSystem::RemoveEntity(DAVA::Entity * removedPoint)
{
    DAVA::FindAndRemoveExchangingWithLast(waypointEntities, removedPoint);
}

void WayEditSystem::RemovePointsGroup(const EntityGroup &entityGroup)
{
    SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());

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
    SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());
    sceneEditor->Exec(new EntityRemoveCommand(removedPoint));

    DAVA::EdgeComponent* edge;

    // get points aiming at removed point, remove edges
    DAVA::List<DAVA::Entity*> srcPoints;
    for (auto waypoint : waypointEntities)
    {
        uint count = waypoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
        for (uint32 i = 0; i < count; ++i)
        {
            edge = static_cast<EdgeComponent*>(waypoint->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
            DVASSERT(edge);
            if (edge->GetNextEntity() == removedPoint)
            {
                sceneEditor->Exec(new RemoveComponentCommand(waypoint, edge));
                srcPoints.push_back(waypoint);
                break;
            }

        }
    }
    // get points aimed by removed point, remove edges
    DAVA::List<DAVA::Entity*> breachPoints;
    DAVA::Entity* dest;
    uint count = removedPoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
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
            DAVA::EdgeComponent* edge;
            uint count = src->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
            for (uint32 i = 0; i < count; ++i)
            {
                edge = static_cast<EdgeComponent*>(src->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
                DVASSERT(edge);
                if (edge->GetNextEntity() == *breachPoint)
                    return true;
            }
            return false;
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

void WayEditSystem::UnregisterComponent(DAVA::Entity* srcWaypoint, DAVA::Component* component)
{
    if (component->GetType() == DAVA::Component::EDGE_COMPONENT)
    {
        DAVA::EdgeComponent* edgeComponent = static_cast<DAVA::EdgeComponent*>(component);
        DAVA::Entity* breachPoint = edgeComponent->GetNextEntity();

        auto HasEdgeToBreachPoint = [&](DAVA::Entity* src)
        {
            DAVA::EdgeComponent* edge;
            uint count = src->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
            for (uint32 i = 0; i < count; ++i)
            {
                edge = static_cast<EdgeComponent*>(src->GetComponent(DAVA::Component::EDGE_COMPONENT, i));
                DVASSERT(edge);
                if (edge->GetNextEntity() == breachPoint)
                    return true;
            }
            return false;
        };

        if (count_if(waypointEntities.begin(), waypointEntities.end(), HasEdgeToBreachPoint) <= 1)
        {
            DAVA::EdgeComponent *clonedEdge = static_cast<DAVA::EdgeComponent*>(edgeComponent->Clone(srcWaypoint));

            SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());
            sceneEditor->Exec(new AddComponentCommand(srcWaypoint, clonedEdge));
        }
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
        
        if ((DAVA::UIEvent::PHASE_ENDED == event->phase) && (DAVA::UIEvent::BUTTON_1 == event->tid))
        {
            int curKeyModifiers = QApplication::keyboardModifiers();
            if(0 == (curKeyModifiers & Qt::ShiftModifier))
            {   //we need use shift key to add waypoint or edge
                return;
            }

            
            SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());
            Entity * currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
            if(!currentWayParent)
            {   // we need have entity with path component
                return;
            }

            
            ProcessSelection();

            if(selectedWaypoints.Size() != 0)
            {
                if(selectedWaypoints.Size() == 1)
                {
                    Entity *nextEntity = selectedWaypoints.GetEntity(0);
                    
                    EntityGroup entitiesForAddEdges = GetEntitiesForAddEdges(nextEntity);
                    const size_t count = entitiesForAddEdges.Size();

                    if(count)
                    {
                        sceneEditor->BeginBatch(DAVA::Format("Add edges pointed on entity %s", nextEntity->GetName().c_str()));

                        AddEdges(entitiesForAddEdges, nextEntity);
                        
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

EntityGroup WayEditSystem::GetEntitiesForAddEdges(DAVA::Entity *nextEntity)
{
    EntityGroup ret;
    
    const size_t count = prevSelectedWaypoints.Size();
    for(size_t i = 0; i < count; ++i)
    {
        Entity * entity = prevSelectedWaypoints.GetEntity(i);
        if(nextEntity->GetParent() != entity->GetParent())
        {   //we don't allow connect different pathes
            continue;
        }

        bool needAddEdge = true;

        const uint32 compCount = entity->GetComponentCount(Component::EDGE_COMPONENT);
        for(uint32 c = 0; c < compCount; ++c)
        {
            EdgeComponent *e = static_cast<EdgeComponent *>(entity->GetComponent(Component::EDGE_COMPONENT, c));
            if(e->GetNextEntity() == nextEntity)
            {
                needAddEdge = false;
                break;
            }
        }
        
        if(needAddEdge)
        {
            ret.Add(entity);
        }
    }
    
    return ret;
}


void WayEditSystem::AddEdges(const EntityGroup & group, DAVA::Entity *nextEntity)
{
    DVASSERT(nextEntity);
    
    SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());
    const size_t count = group.Size();
    for(size_t i = 0; i < count; ++i)
    {
        Entity * entity = group.GetEntity(i);

        DAVA::EdgeComponent *edge = new DAVA::EdgeComponent();
        edge->SetNextEntity(nextEntity);
        
        sceneEditor->Exec(new AddComponentCommand(entity, edge));
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


