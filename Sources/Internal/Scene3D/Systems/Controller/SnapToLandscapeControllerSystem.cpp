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

#include "SnapToLandscapeControllerSystem.h"

#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Landscape.h"


namespace DAVA
{
    
SnapToLandscapeControllerSystem::SnapToLandscapeControllerSystem(Scene * scene)
    : SceneSystem(scene)
{
}

SnapToLandscapeControllerSystem::~SnapToLandscapeControllerSystem()
{
}

void SnapToLandscapeControllerSystem::AddEntity(Entity * entity)
{
    DVASSERT(GetCamera(entity) != NULL && "Right now system works with camera only");

    Camera *camera = GetCamera(entity);
    positions[camera] = Vector3();
    
    entities.push_back(entity);
}

void SnapToLandscapeControllerSystem::RemoveEntity(Entity * entity)
{
    uint32 size = entities.size();
    for(uint32 i = 0; i < size; ++i)
    {
        if(entities[i] == entity)
        {
            Camera *camera = GetCamera(entity);
            if(camera)
            {
                Map<Camera *, Vector3>::iterator it = positions.find(camera);
                if(it != positions.end())
                {
                    positions.erase(it);
                }
                else
                {
                    DVASSERT(false);
                }
            }
            
            entities[i] = entities[size-1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void SnapToLandscapeControllerSystem::Process(float32 timeElapsed)
{
    const uint32 size = entities.size();
    if(0 == size) return;
    
    Landscape *landscape = FindLandscape(GetScene()); //need to use landscape system to get landscape faster

    for(uint32 i = 0; i < size; ++i)
    {
        SnapToLandscapeControllerComponent *snapController = static_cast<SnapToLandscapeControllerComponent *>(entities[i]->GetComponent(Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT));

        Camera *camera =  GetCamera(entities[i]);
        DVASSERT(snapController && camera);

        if(camera && snapController)
        {
            const Vector3 & pos = camera->GetPosition();
            if (pos != positions[camera])
            {
                const Vector3 & direction = camera->GetDirection();

                Vector3 pointOnLandscape;
                if (landscape && landscape->PlacePoint(pos, pointOnLandscape))
                {
                    pointOnLandscape.z += snapController->heightOnLandscape;
                }
                else
                {
                    pointOnLandscape = pos;
                    pointOnLandscape.z = snapController->heightOnLandscape;
                }

                camera->SetPosition(pointOnLandscape);
                camera->SetDirection(direction);
                
                positions[camera] = pointOnLandscape;
            }
        }
    }
}
    
    
};
