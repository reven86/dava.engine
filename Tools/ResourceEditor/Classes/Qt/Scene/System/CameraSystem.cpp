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

// framework
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Scene.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "../StringConstants.h"

SceneCameraSystem::SceneCameraSystem(DAVA::Scene * scene)
	: SceneSystem(scene)
	, curSceneCamera(NULL)
	, curSpeed(35.0f)
	, curViewAngleZ(0)
	, curViewAngleY(0)
	, maxViewAngle(89.0f)
	, animateToNewPos(false)
	, animateToNewPosTime(0)
{
	// add debug cameras
	// there already can be other cameras in scene
	if(NULL != scene)
	{
		DAVA::Camera *mainCamera = NULL;
		DAVA::Camera *topCamera = NULL;

		mainCamera = new DAVA::Camera();
		mainCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		mainCamera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 0.0f));
		mainCamera->SetTarget(DAVA::Vector3(0.0f, 1.0f, 0.0f));
		mainCamera->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);

		DAVA::Entity *mainCameraEntity = new DAVA::Entity();
		mainCameraEntity->SetName(ResourceEditor::EDITOR_MAIN_CAMERA);
		mainCameraEntity->AddComponent(new DAVA::CameraComponent(mainCamera));
		scene->AddNode(mainCameraEntity);
		scene->AddCamera(mainCamera);

		topCamera = new DAVA::Camera();
		topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		topCamera->SetPosition(DAVA::Vector3(0.0f, 0.0f, 200.0f));
		topCamera->SetTarget(DAVA::Vector3(0.0f, 250.0f, 0.0f));
		topCamera->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);

		DAVA::Entity *topCameraEntity = new DAVA::Entity();
		topCameraEntity->SetName(ResourceEditor::EDITOR_DEBUG_CAMERA);
		topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
		scene->AddNode(topCameraEntity);
		scene->AddCamera(topCamera);

		// set current default camera
		if(NULL == scene->GetCurrentCamera())
		{
			scene->SetCurrentCamera(topCamera);
		}

		SafeRelease(mainCamera);
		SafeRelease(topCamera);
	}

	RecalcCameraViewAngles();
}

SceneCameraSystem::~SceneCameraSystem()
{
	SafeRelease(curSceneCamera);
}

DAVA::Vector3 SceneCameraSystem::GetPointDirection(const DAVA::Vector2 &point)
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

DAVA::Vector3 SceneCameraSystem::GetCameraPosition()
{
	DAVA::Vector3 pos;

	if(NULL != curSceneCamera)
	{
		pos = curSceneCamera->GetPosition();
	}

	return pos;
}

void SceneCameraSystem::SetMoveSeep(DAVA::float32 speed)
{
	curSpeed = speed;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed()
{
	return curSpeed;
}

void SceneCameraSystem::SetViewportRect(const DAVA::Rect &rect)
{
	viewportRect = rect;
}

const DAVA::Rect SceneCameraSystem::GetViewportRect()
{
	return viewportRect;
}

DAVA::Vector2 SceneCameraSystem::GetScreenPos(const DAVA::Vector3 &pos3)
{
	DAVA::Vector2 ret;

	if(NULL != curSceneCamera)
	{
		ret = curSceneCamera->GetOnScreenPosition(pos3, viewportRect);
	}

	return ret;
}

DAVA::Vector3 SceneCameraSystem::GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z)
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

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos, const DAVA::Vector3 &direction /* = DAVA::Vector3 */)
{
	animateToNewPos = true;
	animateToNewPosTime = 0;

	newPos = pos;
	newDir = direction;
}

void SceneCameraSystem::Update(float timeElapsed)
{
	DAVA::Scene *scene = GetScene();
	if(NULL != scene)
	{
		DAVA::Camera* camera = scene->GetCurrentCamera();

		// is current camera in scene changed?
		if(curSceneCamera != camera)
		{
			// remember current scene camera
			SafeRelease(curSceneCamera);
			curSceneCamera = camera;
			SafeRetain(curSceneCamera);

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
				SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
				if(NULL != selectionSystem)
				{
					const EntityGroup* selection = selectionSystem->GetSelection();
					if(NULL != selection)
					{
						MouseMoveCameraPosAroundPoint(selection->GetCommonBbox().GetCenter());
					}
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
	// Nothing to draw
}

void SceneCameraSystem::ProcessCommand(const Command2 *command, bool redo)
{ }

void SceneCameraSystem::ProcessKeyboardMove(DAVA::float32 timeElapsed)
{
	if(NULL != curSceneCamera)
	{
		DAVA::float32 moveSpeed = curSpeed * timeElapsed;        

		if(1) // TODO: check for key modifiers
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
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_LEFT) || kd->IsKeyPressed(DAVA::DVKEY_A))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();
				DAVA::Vector3 left = curSceneCamera->GetLeft();

				pos -= left * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_DOWN) || kd->IsKeyPressed(DAVA::DVKEY_S))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();

				pos -= dir * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
			}

			if(kd->IsKeyPressed(DAVA::DVKEY_RIGHT) || kd->IsKeyPressed(DAVA::DVKEY_D))
			{
				DAVA::Vector3 pos = curSceneCamera->GetPosition();
				DAVA::Vector3 dir = curSceneCamera->GetDirection();
				DAVA::Vector3 left = curSceneCamera->GetLeft();

				pos += left * moveSpeed;
				curSceneCamera->SetPosition(pos);
				curSceneCamera->SetDirection(dir);
			}
		}
	}
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

		curSceneCamera->SetTarget(point);
		curSceneCamera->SetPosition(newPos);
	}
}

void SceneCameraSystem::MoveAnimate(DAVA::float32 timeElapsed)
{
	static const DAVA::float32 animationTime = 3.0f;

	if(NULL != curSceneCamera && animateToNewPos)
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 targ = curSceneCamera->GetTarget();

		if(pos != newPos)
		{
			animateToNewPosTime += timeElapsed;

			DAVA::float32 fnX = animateToNewPosTime / animationTime;
			DAVA::float32 fnY = sin(1.57 * fnX);
			
			DAVA::Vector3 dPos = newPos - pos;
			DAVA::Vector3 dTarg = newDir - targ;

			//dPos = dPos * fnY;
			//dTarg = dTarg * fnY;

			if(dPos.Length() > 0.01f) dPos = dPos * fnY;
			if(dTarg.Length() > 0.01f) dTarg = dTarg * fnY;

			curSceneCamera->SetPosition(pos + dPos);
			curSceneCamera->SetTarget(targ + dTarg);
		}
		else
		{
			animateToNewPos = false;
			animateToNewPosTime = 0;
		}
	}
}
