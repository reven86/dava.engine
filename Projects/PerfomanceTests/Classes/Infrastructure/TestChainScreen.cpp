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

#include "TestChainScreen.h"


TestChainScreen::TestChainScreen(const Vector<BaseTest*>& _testsChain, uint32 _fixedTime, uint32 _fixedFramesCount, float32 _fixedDelta) : 
testsChain(_testsChain), currentTest(nullptr), currentTestIndex(0), testsFinished(false),
	fixedTime(_fixedTime), fixedFramesCount(_fixedFramesCount), fixedDelta(_fixedDelta)
{
	currentTest = testsChain[currentTestIndex];
	currentTest->SetupTest(fixedFramesCount, fixedDelta, fixedTime);
}

TestChainScreen::~TestChainScreen()
{
}

bool TestChainScreen::IsFinished() const
{
	return testsFinished;
}

void TestChainScreen::OnStart()
{
}

void TestChainScreen::OnFinish()
{

}

void TestChainScreen::BeginFrame()
{
	RenderManager::Instance()->BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

	currentTest->BeginFrame();
}

void TestChainScreen::EndFrame()
{
	if (currentTest->IsFinished())
	{
		currentTest->FinishTest();
		currentTest->ReleaseTest();

		currentTestIndex++;

		if (currentTestIndex < testsChain.size())
		{
			currentTest = testsChain[currentTestIndex];
			currentTest->SetupTest(fixedFramesCount, fixedDelta, fixedTime);
		}
		else
		{
			testsFinished = true;
		}
	}
	else
	{
		currentTest->EndFrame();
	}

	RenderManager::Instance()->EndFrame();
	RenderManager::Instance()->ProcessStats();
}

void TestChainScreen::Update(float32 timeElapsed)
{
	currentTest->Update(timeElapsed);
}

void TestChainScreen::Draw()
{
	currentTest->Draw();
}
