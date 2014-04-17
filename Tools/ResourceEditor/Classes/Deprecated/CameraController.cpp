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




#include "CameraController.h"

#include "Qt/Settings/SettingsManager.h"
#include "../Qt/Main/QtUtils.h"


namespace DAVA 
{

CameraController::CameraController(float32 newSpeed)
    :   currScene(NULL)
    ,   selection(NULL)
    ,   speed(newSpeed)
{

}
CameraController::~CameraController()
{
    SafeRelease(currScene);
}

void CameraController::SetScene(Scene *_scene)
{
    SafeRelease(currScene);
    currScene = SafeRetain(_scene);
}

void CameraController::Input(UIEvent *)
{
    
}

void CameraController::SetSpeed(float32 newSpeed)
{
    speed = newSpeed;
}

    
WASDCameraController::WASDCameraController(float32 newSpeed)
    : CameraController(newSpeed)
    , viewZAngle(0)
    , viewYAngle(0)
	, lastCamera(NULL)
{

}

WASDCameraController::~WASDCameraController()
{
    
}
    
void WASDCameraController::Update(float32 timeElapsed)
{
    if (currScene == 0)
		return;
    UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
	Camera * camera = currScene->GetCurrentCamera();

	if(camera != lastCamera)
	{
		lastCamera = camera;
		UpdateAngels(camera);
	}

	if(!tf && camera)
    {
        float32 moveSpeed = speed * timeElapsed;        

		if(!IsKeyModificatorsPressed())
        {
            bool moveUp = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_UP) | 
                           InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_W));
            if(moveUp)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 direction = camera->GetDirection();
                //Logger::FrameworkDebug("oldpos: %f %f %f olddir: %f %f %f", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z);
                
                direction.Normalize();
                pos += direction * moveSpeed;
                camera->SetPosition(pos);
                camera->SetDirection(direction);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
                
                //Logger::FrameworkDebug("newpos: %f %f %f", pos.x, pos.y, pos.z);
            }
 
            bool moveLeft = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_LEFT) | 
                           InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_A));
            if(moveLeft)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 dir = camera->GetDirection();
                Vector3 left = camera->GetLeft();
                
                pos -= left * moveSpeed;
                camera->SetPosition(pos);
                camera->SetDirection(dir);
            }
            
            
            bool moveDown = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_DOWN) | 
                             InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_S));
            if(moveDown)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 direction = camera->GetDirection();
                //Logger::FrameworkDebug("oldpos: %f %f %f olddir: %f %f %f", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z);
                
                pos -= direction * moveSpeed;
                camera->SetPosition(pos);
                camera->SetDirection(direction);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
                //Logger::FrameworkDebug("newpos: %f %f %f olddir: %f %f %f", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z);
            }

            
            bool moveRight = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_RIGHT) | 
                             InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_D));
            if(moveRight)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 dir = camera->GetDirection();
                Vector3 left = camera->GetLeft();
                
                pos += left * moveSpeed;
                camera->SetPosition(pos);
                camera->SetDirection(dir);
            }
        }
    }
    
    CameraController::Update(timeElapsed);
}

#define MAX_ANGLE 89

void WASDCameraController::SetScene(Scene *_scene)
{
	Camera *camera = NULL;

	CameraController::SetScene(_scene);

	if(NULL != currScene)
	{
		camera = currScene->GetCurrentCamera();
	}

	UpdateAngels(camera);
}
	
void WASDCameraController::Input(UIEvent * event)
{
	if (currScene == 0)
		return;
	Camera * camera = currScene->GetCurrentCamera();
    if (!camera)return;
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {   
		if(!IsKeyModificatorsPressed())
		{
			switch (event->tid) 
			{
			case DVKEY_Z:
				{
					LookAtSelection();
					break;
				}
			case DVKEY_T:
				{
					if (!camera)return;

					viewZAngle = 0;
					viewYAngle = MAX_ANGLE;

					Matrix4 mt, mt2;
					mt.CreateRotation(Vector3(0,0,1), DegToRad(viewZAngle));
					mt2.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
					mt2 *= mt;
					Vector3 vect = Vector3(0,0, 200);

					Vector3 position = vect + Vector3(0, 10, 0) * mt2;

					camera->SetTarget(position);
					camera->SetPosition(vect);					
					break;					
				}

			case DVKEY_1:
				{
					SetSpeed(SettingsManager::Instance()->GetValue("CameraSpeedValue_0", SettingsManager::DEFAULT).AsInt32());
					break;
				}

			case DVKEY_2:
				{
					SetSpeed(SettingsManager::Instance()->GetValue("CameraSpeedValue_1", SettingsManager::DEFAULT).AsInt32());
					break;
				}

			case DVKEY_3:
				{
					SetSpeed(SettingsManager::Instance()->GetValue("CameraSpeedValue_2", SettingsManager::DEFAULT).AsInt32());
					break;
				}

			case DVKEY_4:
				{
					SetSpeed(SettingsManager::Instance()->GetValue("CameraSpeedValue_3", SettingsManager::DEFAULT).AsInt32());
					break;
				}

			case DVKEY_9:
				{
					if (speed - 50 >= 0)
					{
						speed -= 50;
					}
					break;
				}

			case DVKEY_0:
				{        
					if (speed + 50 <= 5000)
					{
						speed += 50;
					}
					break;
				}

			default:
				break;
			}		
		}
    } 

	bool altBut3 = (selection && event->tid == UIEvent::BUTTON_3 && IsKeyModificatorPressed(DVKEY_ALT));
	
	
	if(UIEvent::PHASE_BEGAN == event->phase)
	{
		startPt = stopPt = event->point;			

		if (altBut3)
		{
			const Vector3 & pos = camera->GetPosition();
			AABBox3 box = selection->GetWTMaximumBoundingBoxSlow();
			center = box.GetCenter();
			radius = (center - pos).Length();
		}
	}
	else if(UIEvent::PHASE_DRAG == event->phase || UIEvent::PHASE_ENDED == event->phase)
	{
		startPt = stopPt;
		stopPt = event->point;

		if (event->tid == UIEvent::BUTTON_2)
		{
			UpdateCam2But(camera);
		}
		else if (altBut3)
		{
			UpdateCamAlt3But(camera);			
		}
		else if (event->tid == UIEvent::BUTTON_3)
		{
			UpdateCam3But(camera);
		}
	}	
}
    
void WASDCameraController::LookAtSelection()
{
    DVASSERT(currScene);
    
    Camera * camera = currScene->GetCurrentCamera();
    if (!camera)return;

    if ( !selection || GetCamera(selection) || GetLandscape(selection))
    {
        return;
    }
    

    AABBox3 box = selection->GetWTMaximumBoundingBoxSlow();
    if(box.IsEmpty())
    {
        return;
    }
    
    float32 boxSize = ((box.max - box.min).Length());
    
    const Vector3 & pos = camera->GetPosition();
    const Vector3 & targ = camera->GetTarget();
    
    Vector3 dir = targ - pos;
    dir.Normalize();
    
    const Vector3 & c = box.GetCenter();
    
    camera->SetTarget(c);
    camera->SetPosition(c - (dir * (boxSize + camera->GetZNear() * 1.5f)));
}

void WASDCameraController::UpdateAngels(Camera * camera)
{
	if(NULL != camera)
	{
		Vector3 dir = camera->GetDirection();

		Vector2 dirXY(dir.x, dir.y);
		dirXY.Normalize();
		viewZAngle = -(RadToDeg(dirXY.Angle()) - 90.0);

		Vector3 dirXY0(dir.x, dir.y, 0.0f);
		dirXY0.Normalize();

		float32 cosA = dirXY0.DotProduct(dir);
		viewYAngle = RadToDeg(acos(cosA));

		if(viewYAngle > MAX_ANGLE)
			viewYAngle -= 360;

		if(viewYAngle < -MAX_ANGLE)
			viewYAngle += 360;
	}
	else
	{
		viewYAngle = 0;
		viewZAngle = 0;
	}
}

void WASDCameraController::UpdateCam2But(Camera * camera)
{
    if (!camera)return;
    Vector2 dp = stopPt - startPt;
    viewZAngle += dp.x * 0.15f;
    viewYAngle += dp.y * 0.15f;
    
    if(viewYAngle < -MAX_ANGLE)
        viewYAngle = -MAX_ANGLE;
    
    if(viewYAngle > MAX_ANGLE)
        viewYAngle = MAX_ANGLE;			
    
    Matrix4 mt, mt2;
    mt.CreateRotation(Vector3(0.f,0.f,1.f), DegToRad(viewZAngle));
    mt2.CreateRotation(Vector3(1.f,0.f,0.f), DegToRad(viewYAngle));
    mt2 *= mt;
    
    Vector3 dir = Vector3(0.f, 10.f, 0.f) * mt2;
    camera->SetDirection(dir);
}

void WASDCameraController::UpdateCamAlt3But(Camera * camera)
{		
    if (!camera)return;
    viewZAngle += (stopPt.x - startPt.x);
    viewYAngle += (stopPt.y - startPt.y);
    
    
    if(viewYAngle < -MAX_ANGLE)
        viewYAngle = -MAX_ANGLE;
    
    if(viewYAngle > MAX_ANGLE)
        viewYAngle = MAX_ANGLE;


    Matrix4 mt, mt2;
    mt.CreateRotation(Vector3(0,0,1), DegToRad(viewZAngle));
    mt2.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
    mt2 *= mt;
    
    Vector3 position = center - Vector3(0, radius, 0) * mt2;
    
    camera->SetTarget(center);
    camera->SetPosition(position);
}

void WASDCameraController::UpdateCam3But(Camera * camera)
{
    if (!camera)return;
    Vector2 dp = stopPt - startPt;

    Matrix4 mt, mt1, mt2, mt3;
    mt1.CreateTranslation(Vector3(-dp.x * 0.15f, 0.f, dp.y * 0.1f));
    mt2.CreateRotation(Vector3(1.f,0.f,0.f), DegToRad(viewYAngle));
    mt3.CreateRotation(Vector3(0.f,0.f,1.f), DegToRad(viewZAngle));
    
    mt = mt1 * mt2 * mt3;
    
    Vector3 pos = camera->GetPosition() + (Vector3(0, 0, 0) * mt);
    Vector3 dir = camera->GetDirection();
    camera->SetPosition(pos);
    camera->SetDirection(dir);
}
    
};
