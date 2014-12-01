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

#include "RotationControllerSystem.h"

#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "UI/UIEvent.h"



namespace DAVA
{
const float32 RotationControllerSystem::maxViewAngle = 89.0f;
    
    
RotationControllerSystem::RotationControllerSystem(Scene * scene)
    : SceneSystem(scene)
    , rotationSpeed(0.15f)
    , curViewAngleZ(0)
    , curViewAngleY(0)
{
    inputCallback = new InputCallback(this, &RotationControllerSystem::Input, InputSystem::INPUT_DEVICE_TOUCH);
//    InputSystem::Instance()->AddInputCallback(*inputCallback);
}

RotationControllerSystem::~RotationControllerSystem()
{
//    InputSystem::Instance()->RemoveInputCallback(*inputCallback);
    SafeDelete(inputCallback);
}

void RotationControllerSystem::AddEntity(Entity * entity)
{
    DVASSERT(GetCamera(entity) != NULL && "Right now system works with camera only");

    entities.push_back(entity);
}

void RotationControllerSystem::RemoveEntity(Entity * entity)
{
    uint32 size = entities.size();
    for(uint32 i = 0; i < size; ++i)
    {
        if(entities[i] == entity)
        {
            entities[i] = entities[size-1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void RotationControllerSystem::Process(float32 timeElapsed)
{
}

void RotationControllerSystem::Input(UIEvent *event)
{
    const uint32 size = entities.size();
    if(0 == size) return;

    if(event->tid == DAVA::UIEvent::BUTTON_2)
    {
        if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
        {
            rotateStartPoint = event->point;
            rotateStopPoint = event->point;
        }
        else if(DAVA::UIEvent::PHASE_DRAG == event->phase || DAVA::UIEvent::PHASE_ENDED == event->phase)
        {
            rotateStartPoint = rotateStopPoint;
            rotateStopPoint = event->point;
            
            //TODO: this code need to check if system used correct for camera and wasd
            Camera * camera = GetScene()->GetDrawCamera();
            if(!camera) return;
            
            //Find active wasd component
            Entity * cameraHolder = NULL;
            for(uint32 i = 0; i < size; ++i)
            {
                if(GetCamera(entities[i]) == camera)
                {
                    cameraHolder = entities[i];
                    break;
                }
            }
            
            DVASSERT(cameraHolder);
            
            RotationControllerComponent *rotationController = static_cast<RotationControllerComponent *>(cameraHolder->GetComponent(Component::ROTATION_CONTROLLER_COMPONENT));
            DVASSERT(rotationController);
            //end of TODO
            
            Rotate(camera);
        }
    }
}

void RotationControllerSystem::Rotate(Camera * camera)
{
    if(NULL != camera && !camera->GetIsOrtho())
    {
        DAVA::Vector2 dp = rotateStopPoint - rotateStartPoint;
        curViewAngleZ += dp.x * rotationSpeed;
        curViewAngleY = Clamp(curViewAngleY + dp.y * rotationSpeed, -maxViewAngle, maxViewAngle);
        
        DAVA::Matrix4 mt, mt2;
        mt.CreateRotation(DAVA::Vector3(0.f,0.f,1.f), DAVA::DegToRad(curViewAngleZ));
        mt2.CreateRotation(DAVA::Vector3(1.f,0.f,0.f), DAVA::DegToRad(curViewAngleY));
        mt2 *= mt;
        
        DAVA::Vector3 dir = DAVA::Vector3(0.f, 10.f, 0.f) * mt2;
        camera->SetDirection(dir);
    }
}


};
