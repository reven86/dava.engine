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

#include "UniversalTest.h"

const FastName UniversalTest::CAMERA_PATH = FastName("CameraPath");
const FastName UniversalTest::TANK_STUB = FastName("TankStub");
const FastName UniversalTest::TANKS = FastName("Tanks");
const FastName UniversalTest::CAMERA = FastName("Camera");

const String UniversalTest::TEST_NAME = "UniversalTest";

const float32 UniversalTest::TANK_ROTATION_ANGLE = 45.0f;

UniversalTest::UniversalTest(const TestParams& params)
    :   BaseTest(TEST_NAME, params)
    ,   camera(new Camera())
    ,   time(0.0f)
{
}

void UniversalTest::LoadResources()
{
    BaseTest::LoadResources();
    
    SceneFileV2::eError error = GetScene()->LoadScene(FilePath("~res:/3d/Maps/" + GetParams().scenePath));
    DVASSERT_MSG(error == SceneFileV2::eError::ERROR_NO_ERROR, ("can't load scene " + GetParams().scenePath).c_str());
    
    Entity* cameraEntity = GetScene()->FindByName(CAMERA);
    
    if(cameraEntity != nullptr)
    {
        Camera* camera = static_cast<CameraComponent*>(cameraEntity->GetComponent(Component::CAMERA_COMPONENT))->GetCamera();
        GetScene()->SetCurrentCamera(camera);
    }
    else
    {
        Entity* cameraPathEntity = GetScene()->FindByName(CAMERA_PATH);
        DVASSERT_MSG(cameraPathEntity != nullptr, "Can't get path component");
        
        PathComponent* pathComponent = static_cast<PathComponent*>(cameraPathEntity->GetComponent(Component::PATH_COMPONENT));
        
        const Vector3& startPosition = pathComponent->GetStartWaypoint()->position;
        const Vector3& destinationPoint = pathComponent->GetStartWaypoint()->edges[0]->destination->position;
        
        camera->SetPosition(startPosition);
        camera->SetTarget(destinationPoint);
        camera->SetUp(Vector3::UnitZ);
        camera->SetLeft(Vector3::UnitY);
        
        GetScene()->SetCurrentCamera(camera);
        
        waypointInterpolator = std::unique_ptr<WaypointsInterpolator>(new WaypointsInterpolator(pathComponent->GetPoints(), GetParams().targetTime / 1000.0f));
    }
    
    Entity* tanksEntity = GetScene()->FindByName(TANKS);
    
    if(tanksEntity != nullptr)
    {
        Vector<Entity*> tanks;
        
        uint32 childrenCount = tanksEntity->GetChildrenCount();
        
        for (uint32 i = 0; i < childrenCount; i++)
        {
            tanks.push_back(tanksEntity->GetChild(i));
        }
        
        for (auto *tank : tanks)
        {
            Vector<uint16> jointsInfo;
            
            TankUtils::MakeSkinnedTank(tank, jointsInfo);
            skinnedTankData.insert(std::pair<FastName, std::pair<Entity*, Vector<uint16>>>(tank->GetName(), std::pair<Entity*, Vector<uint16>>(tank, jointsInfo)));
        }
        
        GetScene()->FindNodesByNamePart(TANK_STUB.c_str(), tankStubs);
        
        auto tankIt = skinnedTankData.cbegin();
        auto tankEnd = skinnedTankData.cend();
        
        for (auto *tankStub : tankStubs)
        {
            Entity* tank = tankIt->second.first;
            
            tankStub->AddNode(ScopedPtr<Entity>(tank->Clone()));
            
            tankIt++;
            if (tankIt == tankEnd)
            {
                tankIt = skinnedTankData.cbegin();
            }
        }
    }
}

void UniversalTest::PerformTestLogic(float32 timeElapsed)
{
    time += timeElapsed;
    
    if(waypointInterpolator)
    {
        waypointInterpolator->NextPosition(camPos, camDst, timeElapsed);
        
        camera->SetPosition(camPos);
        camera->SetTarget(camDst);
    }
    
    for (auto *tank : tankStubs)
    {
        const Vector<uint16>& jointIndexes = skinnedTankData.at(tank->GetChild(0)->GetName()).second;
        TankUtils::Animate(tank, jointIndexes, DegToRad(time * TANK_ROTATION_ANGLE));
    }
}

