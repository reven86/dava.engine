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
#include "Scene3D/Components/UserComponent.h"
#include "Settings/SettingsManager.h"
#include "Scene/SceneEditor2.h"
#include "Commands2/EntityAddCommand.h"

WayEditSystem::WayEditSystem(DAVA::Scene * scene, SceneSelectionSystem *_selectionSystem, SceneCollisionSystem *_collisionSystem)
: DAVA::SceneSystem(scene)
, isEnabled(false)
, inAddNewWayPointState(false)
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

}

void WayEditSystem::Process(DAVA::float32 timeElapsed)
{
    if (isEnabled && NULL == currentWayPoint)
    {
        // draw this collision point
        collisionSystem->SetDrawMode(collisionSystem->GetDrawMode() | CS_DRAW_LAND_COLLISION);
    }
}

void WayEditSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
    if (isEnabled)
    {
        DAVA::Vector3 lanscapeIntersectionPos;
        bool lanscapeIntersected = false;

        currentWayPoint = NULL;

        // check if mouse cursor is under waypoint
        const EntityGroup* collObjects = collisionSystem->ObjectsRayTestFromCamera();
        if (NULL != collObjects && collObjects->Size() > 0)
        {
            DAVA::Entity *underEntity = collObjects->GetEntity(0);
            if (underEntity->GetComponent(Component::USER_COMPONENT) &&
                underEntity->GetParent() == currentWayParent)
            {
                currentWayPoint = underEntity;
            }
        }

        // check if mouse cursor is under landscape point
        lanscapeIntersected = collisionSystem->LandRayTestFromCamera(lanscapeIntersectionPos);

        // process incoming event
        if (event->phase == DAVA::UIEvent::PHASE_BEGAN && event->tid == DAVA::UIEvent::BUTTON_1)
        {
            SceneEditor2* sceneEditor = (SceneEditor2 *)GetScene();
            if (!sceneEditor->modifSystem->InModifState())
            {
                inAddNewWayPointState = true;
            }
        }
        else if (event->phase == DAVA::UIEvent::PHASE_ENDED && event->tid == DAVA::UIEvent::BUTTON_1)
        {
            if (inAddNewWayPointState)
            {
                // mouse key was released under waypoint entity?
                if (NULL != currentWayPoint)
                {
                    int curKeyModifiers = QApplication::keyboardModifiers();

                    // copy waypoint
                    if (curKeyModifiers & Qt::ShiftModifier)
                    {
                        // TODO:
                        // ...
                    }
                    // remove waypoint
                    else if (curKeyModifiers & Qt::ShiftModifier)
                    {
                        RemWayPoint(currentWayPoint);
                        currentWayPoint = NULL;
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
                    // add new waypoint on the landscape
                    if (lanscapeIntersected)
                    {
                        currentWayPoint = AddWayPoint(currentWayParent, lanscapeIntersectionPos);
                    }
                }
            }

            inAddNewWayPointState = false;
        }
    }
}

void WayEditSystem::Draw()
{
    if (NULL != currentWayPoint)
    {
        RenderManager::SetDynamicParam(PARAM_WORLD, &currentWayPoint->GetWorldTransform(), (pointer_size)&currentWayPoint->GetWorldTransform());

        AABBox3 worldBox = selectionSystem->GetSelectionAABox(currentWayPoint);

        DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0.0f, 0.0f, 0.5f));
        DAVA::RenderHelper::Instance()->FillBox(worldBox, wayDrawState);
        DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0.0f, 0.0f, 1.0f));
        DAVA::RenderHelper::Instance()->DrawBox(worldBox, 1.0f, wayDrawState);
    }

    if (NULL != currentWayParent)
    {
    }
}

void WayEditSystem::EnableWayEdit(bool enable)
{
    if (!isEnabled && enable)
    {
        DAVA::Entity *curEntity = selectionSystem->GetSelectionEntity(0);
        if (NULL != curEntity && NULL != curEntity->GetComponent(DAVA::Component::USER_COMPONENT))
        {
            isEnabled = true;
            selectionSystem->SetSelectionAllowed(false);
            currentWayParent = curEntity;
        }
    }
    else
    {
        isEnabled = false;
        selectionSystem->SetSelectionAllowed(true);
    }
}

bool WayEditSystem::IsWayEditEnabled() const
{
    return isEnabled;
}

DAVA::Entity* WayEditSystem::AddWayPoint(DAVA::Entity *parent, DAVA::Vector3 pos)
{
    DAVA::Entity* waypoint = NULL;

    if (NULL != parent && NULL != GetScene())
    {
        waypoint = new DAVA::Entity();
        waypoint->AddComponent(new DAVA::UserComponent());
        waypoint->SetName("waypoint");

        DAVA::Matrix4 pm = currentWayParent->GetWorldTransform();
        pm.Inverse();

        DAVA::Matrix4 m;
        m.SetTranslationVector(pos);
        waypoint->SetLocalTransform(m * pm);

        SceneEditor2 *sceneEditor = ((SceneEditor2 *)GetScene());
        sceneEditor->Exec(new EntityAddCommand(waypoint, currentWayParent));

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
