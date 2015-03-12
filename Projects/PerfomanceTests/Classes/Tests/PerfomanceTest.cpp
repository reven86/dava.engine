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

#include "PerfomanceTest.h"

const String PerfomanceTest::TEST_NAME = "Performance_Test";

PerfomanceTest::PerfomanceTest(uint32 frames, float32 delta, uint32 targetFrame)
    :   BaseTest(TEST_NAME, frames, delta, targetFrame)
    ,   stoneEntity(nullptr)
{
}

PerfomanceTest::PerfomanceTest(uint32 time)
    :   BaseTest(TEST_NAME, time)
    ,   stoneEntity(nullptr)
{
}

void PerfomanceTest::LoadResources()
{
    BaseTest::LoadResources();

    Camera* camera = new Camera();
    camera->SetPosition(Vector3(-40, 4, 60));
    camera->SetTarget(Vector3(-40, -6, 53));
    camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
    camera->SetLeft(Vector3(0.0f, -1.0f, 0.0f));
    camera->SetFOV(70.0f);
    camera->SetZFar(5000);
    camera->SetZNear(1);

    GetScene()->SetCurrentCamera(camera);

    Entity* rootEntity = GetScene()->GetRootNode(FilePath("~res:/3d/Maps/newscene.sc2"));
    GetScene()->AddNode(rootEntity);

    stoneEntity = GetScene()->FindByName("e_stone01.sc2");
}

void PerfomanceTest::UnloadResources()
{
    BaseTest::UnloadResources();
}

void PerfomanceTest::PerformTestLogic(float32 timeElapsed)
{
    const Matrix4& localTransform = stoneEntity->GetLocalTransform();
    Matrix4 newTransform = Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), DAVA::DegToRad(1.0f * timeElapsed));
    
    newTransform *= localTransform;
    stoneEntity->SetLocalTransform(newTransform);
    
    Camera* camera = GetScene()->GetCurrentCamera();
    const Vector3& pos = camera->GetPosition();
    
    Matrix4 cameraMatrix;
    cameraMatrix = Matrix4::MakeTranslation(pos) * Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), DAVA::DegToRad(1.0f));
    camera->SetPosition(cameraMatrix.GetTranslationVector());
}
