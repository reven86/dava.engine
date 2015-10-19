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


#include "WASDControllerSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Utils/Utils.h"

namespace DAVA
{
    
WASDControllerSystem::WASDControllerSystem(Scene * scene)
    : SceneSystem(scene)
    , moveSpeed(1.f)
{
}

WASDControllerSystem::~WASDControllerSystem()
{
}

void WASDControllerSystem::AddEntity(Entity * entity)
{
    DVASSERT(GetCamera(entity) != NULL && "Right now system works with camera only");
    
    entities.push_back(entity);
}

void WASDControllerSystem::RemoveEntity(Entity * entity)
{
    DVVERIFY(FindAndRemoveExchangingWithLast(entities, entity));
}

void WASDControllerSystem::Process(float32 timeElapsed)
{
    float32 actualMoveSpeed = moveSpeed * timeElapsed;

    const uint32 size = static_cast<uint32>(entities.size());
    if(0 == size) return;
    
    KeyboardDevice &keyboard = InputSystem::Instance()->GetKeyboard();
    if(     keyboard.IsKeyPressed(DVKEY_SHIFT)
       ||   keyboard.IsKeyPressed(DVKEY_CTRL)
       ||   keyboard.IsKeyPressed(DVKEY_ALT)
       )
    {
        return;
    }
    
    for(uint32 i = 0; i < size; ++i)
    {
        Camera *camera = GetCamera(entities[i]);
        if ((camera != nullptr) && (camera == GetScene()->GetDrawCamera()))
        {
            if(keyboard.IsKeyPressed(DVKEY_W) || keyboard.IsKeyPressed(DVKEY_UP))
            {
                MoveForward(camera, actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if(keyboard.IsKeyPressed(DVKEY_S) || keyboard.IsKeyPressed(DVKEY_DOWN))
            {
                MoveForward(camera, actualMoveSpeed, DIRECTION_INVERSE);
            }
            if(keyboard.IsKeyPressed(DVKEY_D) || keyboard.IsKeyPressed(DVKEY_RIGHT))
            {
                MoveRight(camera, actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if(keyboard.IsKeyPressed(DVKEY_A) || keyboard.IsKeyPressed(DVKEY_LEFT))
            {
                MoveRight(camera, actualMoveSpeed, DIRECTION_INVERSE);
            }
        }
    }
}


void WASDControllerSystem::MoveForward(Camera *camera, float32 speed, eDirection direction)
{
    Vector3 pos = camera->GetPosition();
    const Vector3 dir = camera->GetDirection();
    
    pos += (dir * speed * (float32)direction);
    
    camera->SetPosition(pos);
    camera->SetDirection(dir);
}
    
void WASDControllerSystem::MoveRight(Camera *camera, float32 speed, eDirection direction)
{
    Vector3 pos = camera->GetPosition();
    const Vector3 dir = camera->GetDirection();
    Vector3 left = camera->GetLeft();
    
    pos += (left * speed * (float32)direction);
    
    camera->SetPosition(pos);
    camera->SetDirection(dir);
}

    
};
