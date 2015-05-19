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

#include "GlobalPerformanceTest.h"

const String GlobalPerformanceTest::TEST_NAME = "RudnikiPerformanceTest";
const String GlobalPerformanceTest::CAMERA_PATH = "CameraPath";

GlobalPerformanceTest::GlobalPerformanceTest(const TestParams& params)
    :   BaseTest(TEST_NAME, params)
    ,   camera(nullptr)
{
}

void GlobalPerformanceTest::LoadResources()
{
    BaseTest::LoadResources();

    Entity* rootEntity = GetScene()->GetRootNode(FilePath("~res:/3d/Maps/rudniki/rudniki.sc2"));
    GetScene()->AddNode(rootEntity);

    Entity* cameraPathEntity = rootEntity->FindByName(CAMERA_PATH.c_str());
    PathComponent* pathComponent = static_cast<PathComponent*>(cameraPathEntity->GetComponent(Component::PATH_COMPONENT));

    const Vector3& startPosition = pathComponent->GetStartWaypoint()->position;
    const Vector3& destinationPoint = pathComponent->GetStartWaypoint()->edges[0]->destination->position;

    camera = new Camera();
    camera->SetPosition(startPosition);
    camera->SetTarget(destinationPoint);
    camera->SetUp(Vector3::UnitZ);
    camera->SetLeft(Vector3::UnitX);

    GetScene()->SetCurrentCamera(camera);

    waypointInterpolator = new WaypointsInterpolator(pathComponent->GetPoints(), GetTargetTestTime());
}

void GlobalPerformanceTest::UnloadResources()
{
    BaseTest::UnloadResources();

    delete waypointInterpolator;
}

void GlobalPerformanceTest::PerformTestLogic(float32 timeElapsed)
{
    Vector3 pos;
    Vector3 target; 

    waypointInterpolator->NextPosition(pos, target, timeElapsed);

    camera->SetPosition(pos);
    camera->SetTarget(target);
}
