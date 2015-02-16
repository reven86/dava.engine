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

#include "BaseTest.h"

const float32 BaseTest::FRAME_OFFSET = 1;

BaseTest::BaseTest(const String& _testName) :
		frameNumber(0), fixedDelta(0.0f), 
		targetFramesCount(0), targetTestTime(0),
		testTime(0), startTime(0), testName(_testName)
{
	pScene = new Scene();
}

BaseTest::~BaseTest()
{
	ReleaseTest();
}

void BaseTest::FinishTest()
{
	elapsedTime = SystemTimer::Instance()->FrameStampTimeMS() - startTime;
}

void BaseTest::ReleaseTest()
{
	SafeRelease(pScene);
}

void BaseTest::SetupTest(uint32 framesCount, float32 fixedDelta, uint32 maxTestTime)
{
	this->fixedDelta = fixedDelta;
	this->targetFramesCount = framesCount;
	this->targetTestTime = maxTestTime;
}

void BaseTest::Draw()
{
	pScene->Draw();
	frameNumber++;
}

void BaseTest::Update(float32 timeElapsed)
{
	if (frameNumber > FRAME_OFFSET)
	{
		if (fixedDelta > 0)
		{
			pScene->Update(fixedDelta);
		}
		else
		{
			pScene->Update(timeElapsed);
		}
		
		frames.push_back(FrameInfo(timeElapsed, frameNumber));
		testTime += timeElapsed;
	}
	else
	{
		pScene->Update(0.0f);
	}	
}

void BaseTest::BeginFrame()
{
	if (frameNumber > 0 && startTime == 0)
	{
		startTime = SystemTimer::Instance()->FrameStampTimeMS();
	}
}

void BaseTest::EndFrame()
{

}