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

PerfomanceTest::PerfomanceTest() : 
	BaseTest(TEST_NAME)
{
}
PerfomanceTest::~PerfomanceTest()
{
}

void PerfomanceTest::SetupTest(uint32 framesCount, float32 fixedDelta, uint32 maxTestTime)
{
	BaseTest::SetupTest(framesCount, fixedDelta, maxTestTime);

	Camera* pCamera = new Camera();
	pCamera->SetPosition(Vector3(-40, 4, 60));
	pCamera->SetTarget(Vector3(-40, -6, 53));
	pCamera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
	pCamera->SetLeft(Vector3(-1.0f, 0.0f, 0.0f));
	pCamera->SetFOV(70.0f);
	pCamera->SetZFar(5000);
	pCamera->SetZNear(1);

	GetScene()->SetCurrentCamera(pCamera);

	Entity* pRootEntity = GetScene()->GetRootNode(FilePath("Data/3d/Maps/newscene.sc2"));
	GetScene()->AddNode(pRootEntity);
}

void PerfomanceTest::Update(float32 timeElapsed)
{
	Entity* pEntity = GetScene()->FindByName("s_stone01.sc2");

	Matrix4 localTransform = pEntity->GetLocalTransform();
	Matrix4 newTransform = Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), DAVA::DegToRad(1.0f));

	newTransform *= localTransform;
	pEntity->SetLocalTransform(newTransform);

	Camera* pCamera = GetScene()->GetCurrentCamera();
	Matrix4 cameraMatrix;
	Vector3 pos = pCamera->GetPosition();
	cameraMatrix = Matrix4::MakeTranslation(pos) * Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), DAVA::DegToRad(1.0f));
	pCamera->SetPosition(cameraMatrix.GetTranslationVector());

	BaseTest::Update(timeElapsed);
}
