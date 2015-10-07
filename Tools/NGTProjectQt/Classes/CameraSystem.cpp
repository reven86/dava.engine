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
#include <QDebug>

#include "CameraSystem.h"
#include "SelectionSystem.h"
#include "CollisionSystem.h"
#include "SceneUtils.h"

// framework
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Scene.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"

#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"

#include <QDebug>


namespace
{
    const auto wheelAdjust = 0.002;
}

SceneCameraSystem::SceneCameraSystem(DAVA::Scene * scene)
	: SceneSystem(scene)
	, debugCamerasCreated(false)
	, curSceneCamera(nullptr)
	, animateToNewPos(false)
	, animateToNewPosTime(0)
	, distanceToCamera(0.f)
	, activeSpeedIndex(0)
{
    renderState = DAVA::RenderManager::Instance()->Subclass3DRenderState(DAVA::RenderStateData::STATE_COLORMASK_ALL | DAVA::RenderStateData::STATE_DEPTH_WRITE);
}

SceneCameraSystem::~SceneCameraSystem()
{
	SafeRelease(curSceneCamera);
}

DAVA::Camera* SceneCameraSystem::GetCurCamera() const
{
    return curSceneCamera;
}

DAVA::Vector3 SceneCameraSystem::GetPointDirection(const DAVA::Vector2 &point) const
{
	DAVA::Vector3 dir;

	if (nullptr != curSceneCamera)
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		dir = curSceneCamera->UnProject(point.x, point.y, 0, viewportRect);
		dir -= pos;
	}

	return dir;
}

DAVA::Vector3 SceneCameraSystem::GetCameraPosition() const
{
	DAVA::Vector3 pos;

	if (nullptr != curSceneCamera)
	{
		pos = curSceneCamera->GetPosition();
	}

	return pos;
}

DAVA::Vector3 SceneCameraSystem::GetCameraDirection() const
{
	DAVA::Vector3 dir;

    if (nullptr != curSceneCamera)
	{
		dir = curSceneCamera->GetDirection();
	}

	return dir;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed()
{
    DAVA::float32 speed = 10.0;

    /*switch(activeSpeedIndex)
    {
    case 0: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed0).AsFloat(); break;
    case 1: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed1).AsFloat(); break;
    case 2: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed2).AsFloat(); break;
    case 3: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed3).AsFloat(); break;
    }*/

	return speed;
}

DAVA::uint32 SceneCameraSystem::GetActiveSpeedIndex()
{
	return activeSpeedIndex;
}

void SceneCameraSystem::SetMoveSpeedArrayIndex(DAVA::uint32 index)
{
	DVASSERT(index < 4);
	activeSpeedIndex = index;
}

void SceneCameraSystem::SetViewportRect(const DAVA::Rect &rect)
{
	viewportRect = rect;

	RecalcCameraAspect();
}

const DAVA::Rect& SceneCameraSystem::GetViewportRect() const
{
	return viewportRect;
}

DAVA::Vector2 SceneCameraSystem::GetScreenPos(const DAVA::Vector3 &pos3) const
{
	DAVA::Vector3 ret3d = GetScreenPosAndDepth(pos3);
	return DAVA::Vector2(ret3d.x, ret3d.y);
}

DAVA::Vector3 SceneCameraSystem::GetScreenPosAndDepth(const DAVA::Vector3 &pos3) const
{
	DAVA::Vector3 ret;

	if (nullptr != curSceneCamera)
	{
		ret = curSceneCamera->GetOnScreenPositionAndDepth(pos3, viewportRect);
	}

	return ret;
}

DAVA::Vector3 SceneCameraSystem::GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z) const
{
	DAVA::Vector3 ret;

    if (nullptr != curSceneCamera)
	{
		ret = curSceneCamera->UnProject(x, y, z, viewportRect);
	}

	return ret;
}

void SceneCameraSystem::LookAt(const DAVA::AABBox3 &box)
{
	if (nullptr != curSceneCamera && !box.IsEmpty())
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 targ = curSceneCamera->GetTarget();
		DAVA::Vector3 dir = targ - pos;
		dir.Normalize();

		DAVA::float32 boxSize = ((box.max - box.min).Length());
        const DAVA::Vector3 c = box.GetCenter();

		pos = c - (dir * (boxSize + curSceneCamera->GetZNear() * 1.5f));
		targ = c;

		MoveTo(pos, targ);
	}
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos)
{
    if (nullptr != curSceneCamera)
	{
		MoveTo(pos, curSceneCamera->GetTarget());
	}
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos, const DAVA::Vector3 &target)
{
    if (nullptr != curSceneCamera && !curSceneCamera->GetIsOrtho())
    {
        animateToNewPos = true;
        animateToNewPosTime = 0;
        
        newPos = pos;
        newTar = target;
    }
}

void SceneCameraSystem::Process(float timeElapsed)
{
    DAVA::Scene * scene = GetScene();
    DAVA::WASDControllerSystem *wasdSystem = findSystem<DAVA::WASDControllerSystem>(scene);
    if(wasdSystem)
    {
        wasdSystem->SetMoveSpeed((animateToNewPos) ? 0 : GetMoveSpeed());
    }

    DAVA::RotationControllerSystem *rotationSystem = findSystem<DAVA::RotationControllerSystem>(scene);
    if(rotationSystem)
        rotationSystem->SetRotationSpeeed((animateToNewPos) ? 0 : 0.15f);
    //TODO: set move speed

    
	if(!debugCamerasCreated)
	{
		CreateDebugCameras();
	}

	if (nullptr != scene)
	{
		DAVA::Camera* camera = scene->GetDrawCamera();

		// is current camera in scene changed?
		if(curSceneCamera != camera)
		{
			// update collision object for last camera
			if(nullptr != curSceneCamera)
			{
				SceneCollisionSystem *collSystem = findSystem<SceneCollisionSystem>(GetScene());
				collSystem->UpdateCollisionObject(GetEntityFromCamera(curSceneCamera));
			}
			
			// remember current scene camera
			SafeRelease(curSceneCamera);
			curSceneCamera = camera;
			SafeRetain(curSceneCamera);

			// Recalc camera aspect
			RecalcCameraAspect();
		}
	}

	// camera move animation
	MoveAnimate(timeElapsed);
}

void SceneCameraSystem::Input(DAVA::UIEvent *event)
{
    switch ( event->phase )
    {
    case DAVA::UIEvent::PHASE_KEYCHAR:
        OnKeyboardInput( event );
        break;
    case DAVA::UIEvent::PHASE_WHEEL:
        break;

    default:
        break;
    }
}

void SceneCameraSystem::OnKeyboardInput( DAVA::UIEvent* event )
{
    const auto isModificatorPressed =
        DAVA::InputSystem::Instance()->GetKeyboard().IsKeyPressed(DAVA::DVKEY_CTRL) ||
        DAVA::InputSystem::Instance()->GetKeyboard().IsKeyPressed(DAVA::DVKEY_ALT) ||
        DAVA::InputSystem::Instance()->GetKeyboard().IsKeyPressed(DAVA::DVKEY_SHIFT);
    if ( isModificatorPressed )
        return;

    switch ( event->tid )
    {
    case DAVA::DVKEY_ADD:
    case DAVA::DVKEY_EQUALS:
        {
            auto entity = GetEntityWithEditorCamera();
            /*auto snapComponent = GetSnapToLandscapeControllerComponent( entity );
            if ( snapComponent != nullptr )
            {
            const auto height = snapComponent->GetHeightOnLandscape() + SettingsManager::Instance()->GetValue( Settings::Scene_CameraHeightOnLandscapeStep ).AsFloat();
            snapComponent->SetHeightOnLandscape( height );
            SettingsManager::Instance()->SetValue( Settings::Scene_CameraHeightOnLandscape, DAVA::VariantType( height ) );
            }*/
        }
        break;
    case DAVA::DVKEY_SUBTRACT:
    case DAVA::DVKEY_MINUS:
        {
            auto entity = GetEntityWithEditorCamera();
            /*auto snapComponent = GetSnapToLandscapeControllerComponent( entity );
            if ( snapComponent != nullptr )
            {
            const auto height = snapComponent->GetHeightOnLandscape() - SettingsManager::Instance()->GetValue( Settings::Scene_CameraHeightOnLandscapeStep ).AsFloat();
            snapComponent->SetHeightOnLandscape( height );
            SettingsManager::Instance()->SetValue( Settings::Scene_CameraHeightOnLandscape, DAVA::VariantType( height ) );
            }*/
        }
        break;

    case DAVA::DVKEY_T:
        MoveTo(DAVA::Vector3(0, 0, 200), DAVA::Vector3(1, 0, 0));
        break;

    case DAVA::DVKEY_1:
        SetMoveSpeedArrayIndex( 0 );
        break;
    case DAVA::DVKEY_2:
        SetMoveSpeedArrayIndex( 1 );
        break;
    case DAVA::DVKEY_3:
        SetMoveSpeedArrayIndex( 2 );
        break;
    case DAVA::DVKEY_4:
        SetMoveSpeedArrayIndex( 3 );
        break;

    default:
        break;
    }
}

void SceneCameraSystem::Draw()
{
    DAVA::Scene * scene = GetScene();
	if(nullptr != scene)
	{
		SceneCollisionSystem *collSystem = findSystem<SceneCollisionSystem>(scene);

		if(nullptr != collSystem)
		{
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));

			DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.begin();
			for(; it != sceneCameras.end(); ++it)
			{
				DAVA::Entity *entity = *it;
                DAVA::Camera *camera = DAVA::GetCamera(entity);

				if(nullptr != entity && nullptr != camera && camera != curSceneCamera)
				{
                    DAVA::AABBox3 worldBox;
                    DAVA::AABBox3 collBox = collSystem->GetBoundingBox(*it);
                    DAVA::Matrix4 transform;

					transform.Identity();
					transform.SetTranslationVector(camera->GetPosition());
					collBox.GetTransformedBox(transform, worldBox);	
					DAVA::RenderHelper::Instance()->FillBox(worldBox, renderState);
				}
			}

			DAVA::RenderManager::Instance()->ResetColor();
		}
	}
}

void SceneCameraSystem::AddEntity(DAVA::Entity * entity)
{
	DAVA::Camera *camera = GetCamera(entity);
	if(nullptr != camera)
	{
		sceneCameras.insert(entity);
	}
}

void SceneCameraSystem::RemoveEntity(DAVA::Entity * entity)
{
	DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.find(entity);
	if(it != sceneCameras.end())
	{
		sceneCameras.erase(it);
	}
}


void SceneCameraSystem::CreateDebugCameras()
{
	DAVA::Scene *scene = GetScene();

	// add debug cameras
	// there already can be other cameras in scene
	if(nullptr != scene)
	{
		DAVA::Camera *topCamera = new DAVA::Camera();
		topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		topCamera->SetPosition(DAVA::Vector3(-50.0f, 0.0f, 50.0f));
		topCamera->SetTarget(DAVA::Vector3(0.0f, 0.1f, 0.0f));
		DAVA::float32 cameraFov = 70.0f;
		DAVA::float32 cameraNear = 1.0f;
		DAVA::float32 cameraFar = 5000.0f;
		topCamera->SetupPerspective(cameraFov, 320.0f / 480.0f, cameraNear, cameraFar);
		topCamera->SetAspect(1.0f);

		DAVA::Entity *topCameraEntity = new DAVA::Entity();
		topCameraEntity->SetName("NGTCamera");
		topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
        topCameraEntity->AddComponent(new DAVA::WASDControllerComponent());
        topCameraEntity->AddComponent(new DAVA::RotationControllerComponent());
        if(scene->GetChildrenCount() > 0)
        {
            scene->InsertBeforeNode(topCameraEntity, scene->GetChild(0));
        }
        else
        {
            scene->AddNode(topCameraEntity);
        }

		// set current default camera
		if(nullptr == scene->GetCurrentCamera())
		{
			scene->SetCurrentCamera(topCamera);
		}
        
        scene->AddCamera(topCamera);

		SafeRelease(topCamera);

		debugCamerasCreated = true;
	}
}

void SceneCameraSystem::RecalcCameraAspect()
{
	if(nullptr != curSceneCamera)
	{
		DAVA::float32 aspect = 1.0;

		if(0 != viewportRect.dx && 0 != viewportRect.dy)
		{
			aspect = viewportRect.dx / viewportRect.dy;
		}

		curSceneCamera->SetAspect(aspect);
	}
}



void SceneCameraSystem::MoveAnimate(DAVA::float32 timeElapsed)
{
	static const DAVA::float32 animationTime = 3.0f;
    static const DAVA::float32 animationStopDistance = 1.0f;

	if(nullptr != curSceneCamera && animateToNewPos)
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 tar = curSceneCamera->GetTarget();
        const DAVA::float32 animationDistance = (pos-newPos).Length();
        
        if((pos != newPos || tar != newTar) && (animateToNewPosTime < animationTime) && (animationDistance > animationStopDistance))
		{
			animateToNewPosTime += timeElapsed;

			DAVA::float32 fnX = animateToNewPosTime / animationTime;
			DAVA::float32 fnY = sin(1.57 * fnX);
			
			DAVA::Vector3 dPos = newPos - pos;
			DAVA::Vector3 dTar = newTar - tar;

			if(dPos.Length() > 0.01f) dPos = dPos * fnY;
			if(dTar.Length() > 0.01f) dTar = dTar * fnY;

			curSceneCamera->SetPosition(pos + dPos);
			curSceneCamera->SetTarget(tar + dTar);
		}
		else
		{
			animateToNewPos = false;
			animateToNewPosTime = 0;

			curSceneCamera->SetTarget(newTar);
			curSceneCamera->SetPosition(newPos);

            findSystem<DAVA::RotationControllerSystem>(GetScene())->RecalcCameraViewAngles(curSceneCamera);
		}
        
        UpdateDistanceToCamera();
	}
}

void SceneCameraSystem::UpdateDistanceToCamera()
{
    DAVA::Vector3 center = findSystem<SceneSelectionSystem>(GetScene())->GetSelection().GetCommonBbox().GetCenter();
    
    const DAVA::Camera *cam = GetScene()->GetCurrentCamera();
    if(cam)
    {
        distanceToCamera = (cam->GetPosition() - center).Length();
    }
    else
    {
        distanceToCamera = 0.f;
    }
}

DAVA::float32 SceneCameraSystem::GetDistanceToCamera() const
{
    return distanceToCamera;
}

DAVA::Entity* SceneCameraSystem::GetEntityFromCamera(DAVA::Camera *c) const
{
	DAVA::Entity *ret = nullptr;

	DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.begin();
	for(; it != sceneCameras.end(); ++it)
	{
		DAVA::Entity *entity = *it;
		DAVA::Camera *camera = GetCamera(entity);

		if(camera == c)
		{
			ret = entity;
			break;
		}
	}

	return ret;
}

void SceneCameraSystem::GetRayTo2dPoint(const DAVA::Vector2 &point, DAVA::float32 maxRayLen, DAVA::Vector3 &outPointFrom, DAVA::Vector3 &outPointTo) const
{
    if(nullptr != curSceneCamera)
    {
        DAVA::Vector3 camPos = GetCameraPosition();
        DAVA::Vector3 camDir = GetPointDirection(point);

        if(curSceneCamera->GetIsOrtho())
        {
            outPointFrom = DAVA::Vector3(camDir.x, camDir.y, camPos.z);
            outPointTo = DAVA::Vector3(camDir.x, camDir.y, camPos.z + maxRayLen);
        }
        else
        {
            outPointFrom = camPos;
            outPointTo = outPointFrom + camDir * maxRayLen;
        }
    }
}


DAVA::Entity* SceneCameraSystem::GetEntityWithEditorCamera() const
{
    DAVA::int32 cameraCount = GetScene()->GetCameraCount();
    for (DAVA::int32 i = 0; i < cameraCount; ++i)
    {
        DAVA::Camera *c = GetScene()->GetCamera(i);
        DAVA::Entity *e = GetEntityFromCamera(c);
        if (e && e->GetName() == DAVA::FastName("NGTCamera"))
        {
            return e;
        }
    }
    
    return nullptr;
}


bool SceneCameraSystem::SnapEditorCameraToLandscape(bool snap)
{
    /*DAVA::Entity *entity = GetEntityWithEditorCamera();
    if(!entity) return false;

    SceneEditor2 *scene = static_cast<SceneEditor2 *>(GetScene());
    
    DAVA::SnapToLandscapeControllerComponent *snapComponent = GetSnapToLandscapeControllerComponent(entity);
    if(snap)
    {
        if(!snapComponent)
        {
            DAVA::float32 height = SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscape).AsFloat();
            
            snapComponent = static_cast<SnapToLandscapeControllerComponent *>(Component::CreateByType(Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT));
            snapComponent->SetHeightOnLandscape(height);

            scene->Exec(new AddComponentCommand(entity, snapComponent));
        }
    }
    else if(snapComponent)
    {
        scene->Exec(new RemoveComponentCommand(entity, snapComponent));
    }
    
    return true;*/
    return false;
}

bool SceneCameraSystem::IsEditorCameraSnappedToLandscape() const
{
    /*Entity *entity = GetEntityWithEditorCamera();
    return (GetSnapToLandscapeControllerComponent(entity) != nullptr);*/
    return false;
}

void SceneCameraSystem::MoveToSelection()
{
    /*auto sceneEditor = dynamic_cast<SceneEditor2*>( GetScene() );
    if ( sceneEditor == nullptr )
        return;

    auto selection = sceneEditor->selectionSystem->GetSelection();
    if ( selection.Size() > 0 )
    {
        sceneEditor->cameraSystem->LookAt( selection.GetCommonBbox() );
    }*/
}

void SceneCameraSystem::MoveToStep( int ofs )
{
    const auto pos = GetCameraPosition();
    const auto direction = GetCameraDirection();
    const auto delta = direction * GetMoveSpeed() * ofs * wheelAdjust;
    const auto dest = pos + delta;
    const auto target = dest + direction;

    MoveTo( dest, target );
}
