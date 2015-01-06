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
#include "Commands2/AddComponentCommand.h"
#include "Utils/Utils.h"


WayEditSystem::WayEditSystem(DAVA::Scene * scene, SceneSelectionSystem *_selectionSystem, SceneCollisionSystem *_collisionSystem)
: DAVA::SceneSystem(scene)
, isEnabled(false)
, selectionSystem(_selectionSystem)
, collisionSystem(_collisionSystem)
{
    wayDrawState = DAVA::RenderManager::Instance()->Subclass3DRenderState(DAVA::RenderStateData::STATE_BLEND |
        DAVA::RenderStateData::STATE_COLORMASK_ALL |
        DAVA::RenderStateData::STATE_DEPTH_TEST);
}

WayEditSystem::~WayEditSystem()
{
    waypointEntities.clear();
}

void WayEditSystem::AddEntity(DAVA::Entity * entity)
{
    if(entity->GetComponent(DAVA::Component::WAYPOINT_COMPONENT))
    {
        waypointEntities.push_back(entity);
    }
}
void WayEditSystem::RemoveEntity(DAVA::Entity * entity)
{
    if(entity->GetComponent(DAVA::Component::WAYPOINT_COMPONENT))
    {
        DAVA::FindAndRemoveExchangingWithLast(waypointEntities, entity);
    }
}


void WayEditSystem::Process(DAVA::float32 timeElapsed)
{
    if (isEnabled)
    {
        // draw this collision point
        collisionSystem->SetDrawMode(collisionSystem->GetDrawMode() | CS_DRAW_LAND_COLLISION);
        
        ProcessSelection();
    }
}


void WayEditSystem::ResetSelection()
{
    selectedWaypoints.Clear();
    prevSelectedWaypoints.Clear();
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
            if(entity->GetComponent(Component::WAYPOINT_COMPONENT))
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
        // process incoming event
        if (event->phase == DAVA::UIEvent::PHASE_ENDED && event->tid == DAVA::UIEvent::BUTTON_1)
        {
            SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());
            Entity * currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
            if(!currentWayParent)
                return;

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
                    sceneEditor->BeginBatch("Add Waypoint");

                    Entity *currentWayPoint = CreateWayPoint(currentWayParent, lanscapeIntersectionPos);
                    sceneEditor->Exec(new EntityAddCommand(currentWayPoint, currentWayParent));
                    
                    if(currentWayPoint && prevSelectedWaypoints.Size())
                    {
                        AddEdges(prevSelectedWaypoints, currentWayPoint);
                    }
                    
                    sceneEditor->EndBatch();
                    
                    selectionSystem->SetSelection(currentWayPoint);
                    
                    currentWayPoint->Release();
                }
            }
        }
    }
}

EntityGroup WayEditSystem::GetEntitiesForAddEdges(DAVA::Entity *nextEntity)
{
    EntityGroup ret;
    
    const size_t count = prevSelectedWaypoints.Size();
    for(size_t i = 0; i < count; ++i)
    {
        Entity * entity = prevSelectedWaypoints.GetEntity(i);

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
//    int id = command->GetId();
//
//    switch (id)
//    {
//        case CMDID_COMPONENT_ADD:
//        case CMDID_COMPONENT_REMOVE:
//        case CMDID_ENTITY_ADD:
//        case CMDID_ENTITY_REMOVE:
//        case CMDID_ENTITY_CHANGE_PARENT:
//            ResetSelection();
//            ProcessSelection();
//            break;
//            
//        default:
//            break;
//    }
    
}


void WayEditSystem::Draw()
{
    const uint32 count = waypointEntities.size();
    for(uint32 i = 0; i < count; ++i)
    {
        Entity *e = waypointEntities[i];

        RenderManager::SetDynamicParam(PARAM_WORLD, &e->GetWorldTransform(), (pointer_size)&e->GetWorldTransform());
        
        AABBox3 worldBox = selectionSystem->GetSelectionAABox(e);
        
        float32 redValue = 0.0f;
        float32 greenValue = 0.0f;
        
        if(currentSelection.HasEntity(e))
        {
            redValue = 1.0f;
        }
        else
        {
            greenValue = 1.0f;
        }
        
        DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0.7f, 0.0f, 0.5f));
        DAVA::RenderHelper::Instance()->FillBox(worldBox, wayDrawState);
        DAVA::RenderManager::Instance()->SetColor(DAVA::Color(redValue, greenValue, 0.0f, 1.0f));
        DAVA::RenderHelper::Instance()->DrawBox(worldBox, 1.0f, wayDrawState);
    }
}

void WayEditSystem::EnableWayEdit(bool enable)
{
    ResetSelection();

    isEnabled = enable;
    if(isEnabled)
    {
        selectionSystem->SetSelectionComponentMask((DAVA::uint64)1 << DAVA::Component::WAYPOINT_COMPONENT | (DAVA::uint64)1 << DAVA::Component::PATH_COMPONENT);
    }
    else
    {
        selectionSystem->ResetSelectionComponentMask();
    }
}

bool WayEditSystem::IsWayEditEnabled() const
{
    return isEnabled;
}


