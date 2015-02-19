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

#include "SingleTestScreen.h"


SingleTestScreen::SingleTestScreen(): 
testForRun(nullptr)
{
}

SingleTestScreen::~SingleTestScreen()
{
}

void SingleTestScreen::OnStart(HashMap<String, BaseObject*>& params)
{
	auto it = params.find(TEST_FOR_RUN);
	if (it != params.end())
	{
		testForRun = static_cast<BaseTest*>(it->second);
		testForRun->SetupTest();
	}

	DVASSERT(testForRun != nullptr);
}

void SingleTestScreen::OnFinish(HashMap<String, BaseObject*>& params)
{

}

void SingleTestScreen::BeginFrame()
{
	RenderManager::Instance()->BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

	testForRun->BeginFrame();
}

void SingleTestScreen::EndFrame()
{
	testForRun->EndFrame();
	if (testForRun->IsPerformed())
	{
		testForRun->FinishTest();
	}

	RenderManager::Instance()->EndFrame();
	RenderManager::Instance()->ProcessStats();
}

void SingleTestScreen::Update(float32 timeElapsed)
{
	if (testForRun->IsDebuggable() 
		&& testForRun->GetFrameNumber() > (testForRun->GetDebugFrame() + BaseTest::FRAME_OFFSET))
	{
		testForRun->Update();
	}
	else
	{
		testForRun->Update(timeElapsed);	
	}	
}

void SingleTestScreen::Draw()
{
	testForRun->Draw();
}
