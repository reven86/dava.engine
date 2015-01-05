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
#include "Utils/Utils.h"


WayEditSystem::WayEditSystem(DAVA::Scene * scene, SceneSelectionSystem *_selectionSystem, SceneCollisionSystem *_collisionSystem)
: DAVA::SceneSystem(scene)
, isEnabled(false)
, currentWayParent(NULL)
, currentWayPoint(NULL)
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
    }
}

void WayEditSystem::Input(DAVA::UIEvent *event)
{
    if (isEnabled)
    {
        // process incoming event
        if (event->phase == DAVA::UIEvent::PHASE_ENDED && event->tid == DAVA::UIEvent::BUTTON_1)
        {
            Entity *oldWaypoint = NULL;
            const EntityGroup* collObjects = collisionSystem->ObjectsRayTestFromCamera();
            if (NULL != collObjects && collObjects->Size() > 0)
            {
                DAVA::Entity *underEntity = collObjects->GetEntity(0);
                if (underEntity->GetComponent(Component::WAYPOINT_COMPONENT) &&
                    underEntity->GetParent() == currentWayParent)
                {
                    oldWaypoint = currentWayPoint;
                    currentWayPoint = underEntity;
                }
            }

            // mouse key was released under waypoint entity?
            if (oldWaypoint)
            {
                int curKeyModifiers = QApplication::keyboardModifiers();
                
                // copy waypoint
                if (curKeyModifiers & Qt::ShiftModifier)
                {
                    // TODO:
                    // ...
                }
                // select waypoint
                else
                {
                    selectionSystem->SetSelection(currentWayPoint);
                }
            }
            // if key was released under landscape
            else
            {
                // check if mouse cursor is under landscape point
                DAVA::Vector3 lanscapeIntersectionPos;
                bool lanscapeIntersected = collisionSystem->LandRayTestFromCamera(lanscapeIntersectionPos);
                
                // add new waypoint on the landscape
                if (lanscapeIntersected)
                {
                    oldWaypoint = currentWayPoint;
                    currentWayPoint = AddWayPoint(currentWayParent, lanscapeIntersectionPos);
                    
                    if(currentWayPoint && oldWaypoint)
                    {
                        DAVA::EdgeComponent *edge = new DAVA::EdgeComponent();
                        edge->SetNextEntity(currentWayPoint);
                        
                        oldWaypoint->AddComponent(edge);
                    }
                }
            }
        }
    }
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
        
        if(e == currentWayPoint)
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
    if (!isEnabled && enable)
    {
        SceneEditor2* sceneEditor = (SceneEditor2 *)GetScene();
        currentWayParent = sceneEditor->pathSystem->GetCurrrentPath();
    }
    else
    {
        currentWayParent = NULL;
    }
    
    isEnabled = (currentWayParent != NULL);
    selectionSystem->SetSelectionAllowed(!isEnabled);
}

bool WayEditSystem::IsWayEditEnabled() const
{
    return isEnabled;
}

DAVA::Entity* WayEditSystem::AddWayPoint(DAVA::Entity *parent, DAVA::Vector3 pos)
{
    DAVA::Entity* waypoint = NULL;
    SceneEditor2 *sceneEditor = ((SceneEditor2 *)GetScene());
    if (parent && sceneEditor)
    {
        DAVA::PathComponent *pc = DAVA::GetPathComponent(parent);
        DVASSERT(pc);
        
        waypoint = new DAVA::Entity();
        waypoint->SetName("Waypoint");
        
        DAVA::WaypointComponent *wc = new DAVA::WaypointComponent();
        wc->SetPathName(pc->GetName());
        waypoint->AddComponent(wc);
        
        DAVA::Matrix4 pm = parent->GetWorldTransform();
        pm.Inverse();
        
        DAVA::Matrix4 m;
        m.SetTranslationVector(pos);
        waypoint->SetLocalTransform(m * pm);
        
        sceneEditor->Exec(new EntityAddCommand(waypoint, parent));
        
        waypoint->Release();
    }

    return waypoint;
}

void WayEditSystem::RemWayPoint(DAVA::Entity *waypoint)
{
    SceneEditor2 *sceneEditor = ((SceneEditor2 *)GetScene());
    if (NULL != sceneEditor)
    {
        EntityGroup group;
        group.Add(waypoint);

        sceneEditor->structureSystem->Remove(group);
    }
}
