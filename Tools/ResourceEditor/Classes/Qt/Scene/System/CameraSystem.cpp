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

#include "Scene/SceneEditor2.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Qt/Settings/SettingsManager.h"

// framework
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Scene.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

#include "../StringConstants.h"

#include "../../Main/QtUtils.h"
#include "Qt/Settings/SettingsManager.h"

#define SPEED_ARRAY_SIZE	4

SceneCameraSystem::SceneCameraSystem(DAVA::Scene * scene)
	: SceneSystem(scene)
	, curSceneCamera(NULL)
	, activeSpeedArrayIndex(0)
	, curViewAngleZ(0)
	, curViewAngleY(0)
	, maxViewAngle(89.0f)
	, animateToNewPos(false)
	, animateToNewPosTime(0)
	, debugCamerasCreated(false)
    , distanceToCamera(0.f)
    , cameraShouldIgnoreKeyboard(false)
{
	renderState = RenderManager::Instance()->Subclass3DRenderState(RenderStateData::STATE_COLORMASK_ALL | RenderStateData::STATE_DEPTH_WRITE);
	
	cameraSpeedArray.push_back(SettingsManager::Instance()->GetValue("CameraSpeedValue_0", SettingsManager::DEFAULT).AsFloat());
	cameraSpeedArray.push_back(SettingsManager::Instance()->GetValue("CameraSpeedValue_1", SettingsManager::DEFAULT).AsFloat());
	cameraSpeedArray.push_back(SettingsManager::Instance()->GetValue("CameraSpeedValue_2", SettingsManager::DEFAULT).AsFloat());
	cameraSpeedArray.push_back(SettingsManager::Instance()->GetValue("CameraSpeedValue_3", SettingsManager::DEFAULT).AsFloat());
}

SceneCameraSystem::~SceneCameraSystem()
{
	SafeRelease(curSceneCamera);
}

DAVA::Vector3 SceneCameraSystem::GetPointDirection(const DAVA::Vector2 &point) const
{
	DAVA::Vector3 dir;

	if(NULL != curSceneCamera)
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

	if(NULL != curSceneCamera)
	{
		pos = curSceneCamera->GetPosition();
	}

	return pos;
}

DAVA::Vector3 SceneCameraSystem::GetCameraDirection() const
{
	DAVA::Vector3 dir;

	if(NULL != curSceneCamera)
	{
		dir = curSceneCamera->GetDirection();
	}

	return dir;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed()
{
	return cameraSpeedArray[activeSpeedArrayIndex];
}

DAVA::uint32 SceneCameraSystem::GetActiveSpeedIndex()
{
	return activeSpeedArrayIndex;
}

void SceneCameraSystem::SetMoveSpeedArrayIndex(DAVA::uint32 index)
{
	DVASSERT(index < 4);
	activeSpeedArrayIndex = index;
}

void SceneCameraSystem::SetMoveSpeed(DAVA::float32 speed, DAVA::uint32 index)
{
	DVASSERT(index < 4);
	cameraSpeedArray[index] = speed;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed(uint32 index)
{
	return cameraSpeedArray[index];
}

void SceneCameraSystem::SetViewportRect(const DAVA::Rect &rect)
{
	viewportRect = rect;

	RecalcCameraAspect();
}

const DAVA::Rect SceneCameraSystem::GetViewportRect()
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

	if(NULL != curSceneCamera)
	{
		if(curSceneCamera)
		{
			ret = curSceneCamera->GetOnScreenPositionAndDepth(pos3, viewportRect);
		}
	}

	return ret;
}

DAVA::Vector3 SceneCameraSystem::GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z) const
{
	DAVA::Vector3 ret;

	if(NULL != curSceneCamera)
	{
		ret = curSceneCamera->UnProject(x, y, z, viewportRect);
	}

	return ret;
}

void SceneCameraSystem::LookAt(const DAVA::AABBox3 &box)
{
	if(NULL != curSceneCamera && !box.IsEmpty())
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 targ = curSceneCamera->GetTarget();
		DAVA::Vector3 dir = targ - pos;
		dir.Normalize();

		float32 boxSize = ((box.max - box.min).Length());
		const Vector3 c = box.GetCenter();

		pos = c - (dir * (boxSize + curSceneCamera->GetZNear() * 1.5f));
		targ = c;

		MoveTo(pos, targ);
	}
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos)
{
	if(NULL != curSceneCamera)
	{
		MoveTo(pos, curSceneCamera->GetTarget());
	}
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos, const DAVA::Vector3 &target)
{
	animateToNewPos = true;
	animateToNewPosTime = 0;

	newPos = pos;
	newTar = target;
}

void SceneCameraSystem::Update(float timeElapsed)
{
	if(!debugCamerasCreated)
	{
		CreateDebugCameras();
	}

	DAVA::Scene *scene = GetScene();
	if(NULL != scene)
	{
		DAVA::Camera* camera = scene->GetCurrentCamera();

		// is current camera in scene changed?
		if(curSceneCamera != camera)
		{
			// update collision object for last camera
			if(NULL != curSceneCamera)
			{
				SceneCollisionSystem *collSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
				collSystem->UpdateCollisionObject(GetEntityFromCamera(curSceneCamera));
			}
			
			// remember current scene camera
			SafeRelease(curSceneCamera);
			curSceneCamera = camera;
			SafeRetain(curSceneCamera);

			// Recalc camera aspect
			RecalcCameraAspect();

			// recalc current view angles using new camera pos and direction
			RecalcCameraViewAngles();
		}

		ProcessKeyboardMove(timeElapsed);
	}

	// camera move animation
	MoveAnimate(timeElapsed);
}

void SceneCameraSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	// process move
	if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
	{
		rotateStartPoint = event->point;
		rotateStopPoint = event->point;
	}
	else if(DAVA::UIEvent::PHASE_DRAG == event->phase || DAVA::UIEvent::PHASE_ENDED == event->phase)
	{
		rotateStartPoint = rotateStopPoint;
		rotateStopPoint = event->point;

		if(event->tid == DAVA::UIEvent::BUTTON_2)
		{
			MouseMoveCameraDirection();
		}
		else if(event->tid == DAVA::UIEvent::BUTTON_3)
		{
			if(Qt::AltModifier & QApplication::keyboardModifiers())
			{
				HoodSystem *hoodSystem = ((SceneEditor2 *) GetScene())->hoodSystem;
				if(NULL != hoodSystem)
				{
					MouseMoveCameraPosAroundPoint(hoodSystem->GetPosition());
				}
			}
			else
			{
				MouseMoveCameraPosition();
			}
		}
	}

	// Other event, like camera speed, or switch to next camera
	// ...
}

void SceneCameraSystem::Draw()
{
	//int oldState = DAVA::RenderManager::Instance()->GetState();
	//DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_TEST);
	
	SceneEditor2 *sceneEditor = (SceneEditor2 *) GetScene();
	if(NULL != sceneEditor)
	{
		SceneCollisionSystem *collSystem = sceneEditor->collisionSystem;

		if(NULL != collSystem)
		{
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));		

			DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.begin();
			for(; it != sceneCameras.end(); ++it)
			{
				DAVA::Entity *entity = *it;
				DAVA::Camera *camera = GetCamera(entity);

				if(NULL != entity && NULL != camera && camera != curSceneCamera)
				{
					AABBox3 worldBox;
					AABBox3 collBox = collSystem->GetBoundingBox(*it);
					Matrix4 transform;

					transform.Identity();
					transform.SetTranslationVector(camera->GetPosition());
					collBox.GetTransformedBox(transform, worldBox);	
					DAVA::RenderHelper::Instance()->FillBox(worldBox, renderState);
				}
			}

			DAVA::RenderManager::Instance()->ResetColor();
		}
	}

	//DAVA::RenderManager::Instance()->SetState(oldState);
}

void SceneCameraSystem::ProcessCommand(const Command2 *command, bool redo)
{ }

void SceneCameraSystem::AddEntity(DAVA::Entity * entity)
{
	DAVA::Camera *camera = GetCamera(entity);
	if(NULL != camera)
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

bool SceneCameraSystem::IsCameraMovementKeyPressed()
{
    DAVA::KeyboardDevice *kd = DAVA::InputSystem::Instance()->GetKeyboard();
    bool movingKeyPressed = kd->IsKeyPressed(DAVA::DVKEY_UP) | kd->IsKeyPressed(DAVA::DVKEY_W) |
                            kd->IsKeyPressed(DAVA::DVKEY_LEFT) | kd->IsKeyPressed(DAVA::DVKEY_A) |
                            kd->IsKeyPressed(DAVA::DVKEY_DOWN) | kd->IsKeyPressed(DAVA::DVKEY_S) |
                            kd->IsKeyPressed(DAVA::DVKEY_RIGHT) | kd->IsKeyPressed(DAVA::DVKEY_D);

    return movingKeyPressed;
}

bool SceneCameraSystem::IsModifiersPressed()
{
    return (QApplication::queryKeyboardModifiers() != Qt::NoModifier);
}

void SceneCameraSystem::ProcessKeyboardMove(DAVA::float32 timeElapsed)
{
	if(NULL != curSceneCamera)
	{
		DAVA::float32 moveSpeed = cameraSpeedArray[activeSpeedArrayIndex] * timeElapsed;        

        // since pressing shortcuts with camera movement keys could produce camera flickering,
        // camera should ignore movement till both - modifier keys and movement keys - are released
        if (!IsCameraMovementKeyPressed())
        {
            cameraShouldIgnoreKeyboard = false;
        }
        cameraShouldIgnoreKeyboard |= IsModifiersPressed();

		if(!cameraShouldIgnoreKeyboard)
		{
			DAVA::KeyboardDevice *kd = DAVA::InputSystem::Instance()->GetKeyboard();

			if(kd->IsKeyPressed(DAVA::DVKEY_UP) || kd->IsKeyPressed(DAVA::DVKEY_W))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();

				dir.Normalize();
				pos += dir * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
                UpdateDistanceToCamera();
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_LEFT) || kd->IsKeyPressed(DAVA::DVKEY_A))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();
				DAVA::Vector3 left = curSceneCamera->GetLeft();

				pos -= left * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);
                UpdateDistanceToCamera();
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_DOWN) || kd->IsKeyPressed(DAVA::DVKEY_S))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();

				pos -= dir * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
                UpdateDistanceToCamera();
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_RIGHT) || kd->IsKeyPressed(DAVA::DVKEY_D))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();
				DAVA::Vector3 left = curSceneCamera->GetLeft();

				pos += left * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);
                UpdateDistanceToCamera();
			}
		}
	}
}


void SceneCameraSystem::CreateDebugCameras()
{
	DAVA::Scene *scene = GetScene();

	// add debug cameras
	// there already can be other cameras in scene
	if(NULL != scene)
	{
		DAVA::Camera *mainCamera = NULL;
		DAVA::Camera *topCamera = NULL;

		/*
		mainCamera = new DAVA::Camera();
		mainCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		mainCamera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 0.0f));
		mainCamera->SetTarget(DAVA::Vector3(0.0f, 1.0f, 0.0f));
		mainCamera->SetAspect(1.0f);
		mainCamera->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);

		DAVA::Entity *mainCameraEntity = new DAVA::Entity();
		mainCameraEntity->SetName(ResourceEditor::EDITOR_MAIN_CAMERA);
		mainCameraEntity->AddComponent(new DAVA::CameraComponent(mainCamera));
		scene->InsertBeforeNode(mainCameraEntity, scene->GetChild(0));
		*/

		topCamera = new DAVA::Camera();
		topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		topCamera->SetPosition(DAVA::Vector3(-50.0f, 0.0f, 50.0f));
		topCamera->SetTarget(DAVA::Vector3(0.0f, 0.1f, 0.0f));
		float fov = SettingsManager::Instance()->GetValue(ResourceEditor::SETTINGS_DEFAULT_FOV, SettingsManager::DEFAULT).AsFloat();
		topCamera->SetupPerspective(fov, 320.0f / 480.0f, 1.0f, 5000.0f);
		topCamera->SetAspect(1.0f);

		DAVA::Entity *topCameraEntity = new DAVA::Entity();
		topCameraEntity->SetName(ResourceEditor::EDITOR_DEBUG_CAMERA);
		topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
		scene->InsertBeforeNode(topCameraEntity, scene->GetChild(0));

		// set current default camera
		if(NULL == scene->GetCurrentCamera())
		{
			scene->SetCurrentCamera(topCamera);
		}

		SafeRelease(mainCamera);
		SafeRelease(topCamera);

		debugCamerasCreated = true;
	}

	RecalcCameraViewAngles();
}

void SceneCameraSystem::RecalcCameraViewAngles()
{
	animateToNewPos = false;

	if(NULL != curSceneCamera)
	{
		DAVA::Vector3 dir = curSceneCamera->GetDirection();
		DAVA::Vector2 dirXY(dir.x, dir.y);

		dirXY.Normalize();
		curViewAngleZ = -(DAVA::RadToDeg(dirXY.Angle()) - 90.0f);

		DAVA::Vector3 dirXY0(dir.x, dir.y, 0.0f);
		dirXY0.Normalize();

		DAVA::float32 cosA = dirXY0.DotProduct(dir);
		curViewAngleY = DAVA::RadToDeg(acos(cosA));

		if(curViewAngleY > maxViewAngle)
			curViewAngleY -= 360;

		if(curViewAngleY < -maxViewAngle)
			curViewAngleY += 360;
	}
	else
	{
		curViewAngleY = 0;
		curViewAngleZ = 0;
	}
    
    UpdateDistanceToCamera();
}

void SceneCameraSystem::RecalcCameraAspect()
{
	if(NULL != curSceneCamera)
	{
		DAVA::float32 aspect = 1.0;

		if(0 != viewportRect.dx && 0 != viewportRect.dy)
		{
			aspect = viewportRect.dx / viewportRect.dy;
		}

		curSceneCamera->SetAspect(aspect);
	}
}

void SceneCameraSystem::MouseMoveCameraDirection()
{
	animateToNewPos = false;

	if(NULL != curSceneCamera)
	{
		DAVA::Vector2 dp = rotateStopPoint - rotateStartPoint;
		curViewAngleZ += dp.x * 0.15f;
		curViewAngleY += dp.y * 0.15f;

		if(curViewAngleY < -maxViewAngle)
			curViewAngleY = -maxViewAngle;

		if(curViewAngleY > maxViewAngle)
			curViewAngleY = maxViewAngle;			

		DAVA::Matrix4 mt, mt2;
		mt.CreateRotation(DAVA::Vector3(0.f,0.f,1.f), DAVA::DegToRad(curViewAngleZ));
		mt2.CreateRotation(DAVA::Vector3(1.f,0.f,0.f), DAVA::DegToRad(curViewAngleY));
		mt2 *= mt;

		DAVA::Vector3 dir = DAVA::Vector3(0.f, 10.f, 0.f) * mt2;
		curSceneCamera->SetDirection(dir);
        
        UpdateDistanceToCamera();
	}
}

void SceneCameraSystem::MouseMoveCameraPosition()
{
	animateToNewPos = false;

	if(NULL != curSceneCamera)
	{
		DAVA::Vector2 dp = rotateStopPoint - rotateStartPoint;
		DAVA::Matrix4 mt, mt1, mt2, mt3;

		mt1.CreateTranslation(DAVA::Vector3(-dp.x * 0.15f, 0.f, dp.y * 0.1f));
		mt2.CreateRotation(DAVA::Vector3(1.f,0.f,0.f), DAVA::DegToRad(curViewAngleY));
		mt3.CreateRotation(DAVA::Vector3(0.f,0.f,1.f), DAVA::DegToRad(curViewAngleZ));

		mt = mt1 * mt2 * mt3;

		DAVA::Vector3 pos = curSceneCamera->GetPosition() + (DAVA::Vector3(0, 0, 0) * mt);
		DAVA::Vector3 dir = curSceneCamera->GetDirection();
        
		curSceneCamera->SetPosition(pos);
		curSceneCamera->SetDirection(dir);
        UpdateDistanceToCamera();
	}
}

void SceneCameraSystem::MouseMoveCameraPosAroundPoint(const DAVA::Vector3 &point)
{		
	animateToNewPos = false;

	if(NULL != curSceneCamera)
	{
		curViewAngleZ += (rotateStopPoint.x - rotateStartPoint.x);
		curViewAngleY += (rotateStopPoint.y - rotateStartPoint.y);

		if(curViewAngleY < -maxViewAngle)
			curViewAngleY = -maxViewAngle;

		if(curViewAngleY > maxViewAngle)
			curViewAngleY = maxViewAngle;


		DAVA::Matrix4 mt, mt2;
		mt.CreateRotation(DAVA::Vector3(0,0,1), DAVA::DegToRad(curViewAngleZ));
		mt2.CreateRotation(DAVA::Vector3(1,0,0), DAVA::DegToRad(curViewAngleY));
		mt2 *= mt;

		DAVA::Vector3 curPos = curSceneCamera->GetPosition();
		DAVA::float32 radius = (point - curPos).Length();
		DAVA::Vector3 newPos = point - DAVA::Vector3(0, radius, 0) * mt2;

		curSceneCamera->SetPosition(newPos);
		curSceneCamera->SetTarget(point);
        UpdateDistanceToCamera();
	}
}

void SceneCameraSystem::MoveAnimate(DAVA::float32 timeElapsed)
{
	static const DAVA::float32 animationTime = 3.0f;

	if(NULL != curSceneCamera && animateToNewPos)
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 tar = curSceneCamera->GetTarget();

		if((pos != newPos || tar != newTar) && animateToNewPosTime < animationTime)
		{
			animateToNewPosTime += timeElapsed;

			DAVA::float32 fnX = animateToNewPosTime / animationTime;
			DAVA::float32 fnY = sin(1.57 * fnX);
			
			DAVA::Vector3 dPos = newPos - pos;
			DAVA::Vector3 dTar = newTar - tar;

			//dPos = dPos * fnY;
			//dTarg = dTarg * fnY;

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

			RecalcCameraViewAngles();
		}
        
        UpdateDistanceToCamera();
	}
}

void SceneCameraSystem::UpdateDistanceToCamera()
{
    SceneEditor2 *sc = (SceneEditor2 *)GetScene();
    
    Vector3 center = sc->selectionSystem->GetSelection().GetCommonBbox().GetCenter();
    
    const Camera *cam = GetScene()->GetCurrentCamera();
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
	DAVA::Entity *ret = NULL;

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
