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

#ifndef __BASE_TEST_H__
#define __BASE_TEST_H__

#include "DAVAEngine.h"
#include <list>

using std::list;
using namespace DAVA;

class BaseTest
{
public:
	BaseTest(const String& testName);
	~BaseTest();

	struct FrameInfo
	{
		FrameInfo() {}
		FrameInfo(float32 delta, uint32 frame) : delta(delta), frame(frame) {}
			
		float32 delta;
		uint32 frame;
	};

	virtual void SetupTest(uint32 framesCount, float32 fixedDelta, uint32 maxTestTime);
	virtual void FinishTest();
	virtual void ReleaseTest();

	bool IsFinished() const;

	virtual void BeginFrame();
	virtual void EndFrame();

	virtual void Update(float32 timeElapsed);
	virtual void Draw();

	const List<FrameInfo>& GetFramesInfo() const;
	const String& GetName() const;

	float32 GetTestTime() const;
	uint64 GetElapsedTime() const;

	Scene* GetScene() const;

private:
	List<FrameInfo> frames;
	String testName;
	
	uint32 frameNumber;
	float32 testTime;
	uint64 startTime;
	uint64 elapsedTime;

	uint32 targetFramesCount;
	float32 fixedDelta;

	uint32 targetTestTime;

	Scene* pScene;
};

inline const List<BaseTest::FrameInfo>& BaseTest::GetFramesInfo() const
{
	return frames;
}

inline Scene* BaseTest::GetScene() const
{
	return pScene;
}

inline bool BaseTest::IsFinished() const
{
	if (targetFramesCount > 0 && frameNumber >= (targetFramesCount + 2))
	{
		return true;
	}
	if (targetTestTime > 0 && (testTime * 1000) >= targetTestTime)
	{
		return true;
	}

	return false;
}

inline const String& BaseTest::GetName() const
{
	return testName;
}

inline uint64 BaseTest::GetElapsedTime() const
{
	return elapsedTime;
}

inline float32 BaseTest::GetTestTime() const
{
	return testTime;
}

#endif