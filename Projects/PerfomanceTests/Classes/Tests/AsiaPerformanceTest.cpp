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

#include "AsiaPerformanceTest.h"

const String AsiaPerformanceTest::TEST_NAME = "AsiaPerformanceTest";
const String AsiaPerformanceTest::CAMERA_PATH = "CameraPath";
const String AsiaPerformanceTest::TANK_STUB = "TankStub";

const String AsiaPerformanceTest::CENTURION = "Centurion.sc2";
const String AsiaPerformanceTest::CONQUEROR = "Conqueror.sc2";
const String AsiaPerformanceTest::VALENTINE = "Valentine_LL.sc2";
const String AsiaPerformanceTest::T150 = "T150.sc2";
const String AsiaPerformanceTest::T110E5 = "T110E5.sc2";

const float32 AsiaPerformanceTest::TANK_ROTATION_ANGLE = 45.0f;

AsiaPerformanceTest::AsiaPerformanceTest(const TestParams& params)
    : BaseTest(TEST_NAME, params)
    , camera(nullptr)
    , time(0.0f)
{
}

void AsiaPerformanceTest::LoadResources()
{
    BaseTest::LoadResources();

    Entity* rootEntity = GetScene()->GetRootNode(FilePath("~res:/3d/Maps/10_asia_as/10_asia_as.sc2"));
    GetScene()->AddNode(rootEntity);

    Entity* cameraPathEntity = rootEntity->FindByName(CAMERA_PATH.c_str());
    PathComponent* pathComponent = static_cast<PathComponent*>(cameraPathEntity->GetComponent(Component::PATH_COMPONENT));

    const Vector3& startPosition = pathComponent->GetStartWaypoint()->position;
    const Vector3& destinationPoint = pathComponent->GetStartWaypoint()->edges[0]->destination->position;

    camera = new Camera();
    camera->SetPosition(startPosition);
    camera->SetTarget(destinationPoint);
    camera->SetUp(Vector3::UnitZ);
    camera->SetLeft(Vector3::UnitY);

    GetScene()->SetCurrentCamera(camera);

    waypointInterpolator = new WaypointsInterpolator(pathComponent->GetPoints(),  GetTargetTestTime());
    tankAnimator = new TankAnimator();

    Vector<Entity*> tanks;

    tanks.push_back(rootEntity->FindByName(CENTURION.c_str()));
    tanks.push_back(rootEntity->FindByName(CONQUEROR.c_str()));
    tanks.push_back(rootEntity->FindByName(VALENTINE.c_str()));
    tanks.push_back(rootEntity->FindByName(T150.c_str()));
    tanks.push_back(rootEntity->FindByName(T110E5.c_str()));

    for (uint32 i = 0; i < tanks.size(); i++)
    {
        Entity* tank = tanks[i];
        Vector<uint16> jointsInfo;

        tankAnimator->MakeSkinnedTank(tank, jointsInfo);
        skinnedTankData.insert(std::pair<FastName, std::pair<Entity*, Vector<uint16>>>(tank->GetName(), std::pair<Entity*, Vector<uint16>>(tank, jointsInfo)));
    }
    
    rootEntity->FindNodesByNamePart(TANK_STUB.c_str(), tankStubs);

    auto tankIt = skinnedTankData.cbegin();
    auto tankEnd = skinnedTankData.cend();

    for (Entity* tankStub : tankStubs)
    {
        Entity* tank = tankIt->second.first;
        Entity* newTank = tank->Clone();

        tankStub->AddNode(newTank);

        tankIt++;
        if (tankIt == tankEnd)
        {
            tankIt = skinnedTankData.cbegin();
        }
    }
}

void AsiaPerformanceTest::UnloadResources()
{
    BaseTest::UnloadResources();

    delete waypointInterpolator;
    delete tankAnimator;
}

void AsiaPerformanceTest::PerformTestLogic(float32 timeElapsed)
{
    time += timeElapsed;
    waypointInterpolator->NextPosition(camPos, camDst, timeElapsed);

    camera->SetPosition(camPos);
    camera->SetTarget(camDst);

    for (Entity* tank : tankStubs)
    {
        const Vector<uint16>& jointIndexes = skinnedTankData.at(tank->GetChild(0)->GetName()).second;
        tankAnimator->Animate(tank, jointIndexes, DegToRad(time * TANK_ROTATION_ANGLE));
    }
}
