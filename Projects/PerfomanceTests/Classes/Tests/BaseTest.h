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

#include "Infrastructure/Screen/BaseScreen.h"
#include "Utils/ConverterUtils.h"
#include "TeamCityTestsOutput.h"


class BaseTest : public BaseScreen
{
public:
    
    struct FrameInfo
    {
        FrameInfo() {}
        FrameInfo(float32 delta, uint32 frame) : delta(delta), frame(frame) {}

        float32 delta;
        uint32 frame;
    };

    BaseTest(const String& testName, uint32 frames, float32 delta, uint32 debugFrame);
    BaseTest(const String& testName, uint32 time);
    
    void OnStart() override;
    void OnFinish() override;
    
    void SetDebuggable(bool value);
    bool IsDebuggable() const;
    bool IsFinished() const override;
    
    void BeginFrame() override;
    void EndFrame() override;
    void SystemUpdate(float32 timeElapsed) override;
    
    const List<FrameInfo>& GetFramesInfo() const;
    const String& GetName() const;
    
    float32 GetTestTime() const;
    float32 GetFixedDelta() const;
    uint64 GetElapsedTime() const;
    uint32 GetFrameNumber() const; 
    uint32 GetDebugFrame() const;
    
    Scene* GetScene() const;
    
    static const float32 FRAME_OFFSET;
    
protected:
    
    virtual ~BaseTest() {};

    void LoadResources() override;
    void UnloadResources() override;
    
    virtual void PerformTestLogic(float32 timeElapsed) = 0;
    
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
    uint32 debugFrame;
    
    Scene* scene;
    UI3DView* sceneView;
    
    bool debuggable;
};

inline const List<BaseTest::FrameInfo>& BaseTest::GetFramesInfo() const
{
    return frames;
}

inline Scene* BaseTest::GetScene() const
{
    return scene;
}

inline bool BaseTest::IsFinished() const
{
    if (targetFramesCount > 0 && frameNumber >= (targetFramesCount + FRAME_OFFSET))
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

inline uint32 BaseTest::GetDebugFrame() const
{
    return debugFrame;
}

inline uint32 BaseTest::GetFrameNumber() const
{
    return frameNumber;
}

inline bool BaseTest::IsDebuggable() const
{
    return debuggable;
}

inline void BaseTest::SetDebuggable(bool value)
{
    debuggable = value;
}

inline float32 BaseTest::GetFixedDelta() const
{
    return fixedDelta;
}

#endif