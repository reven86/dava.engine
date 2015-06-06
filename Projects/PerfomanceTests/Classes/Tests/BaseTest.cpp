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

const float32 BaseTest::FRAME_OFFSET = 5;

BaseTest::BaseTest(const String& _testName, const TestParams& testParams)
    :   testName(_testName)
    ,   targetFrameDelta(testParams.targetFrameDelta)
    ,   targetFramesCount(testParams.targetFramesCount)
    ,   targetTestTime(testParams.targetTime)
    ,   frameForDebug(testParams.frameForDebug)
    ,   maxDelta(testParams.maxDelta)
    ,   frameNumber(0)
    ,   testTime(0.0f)
    ,   startTime(0)
    ,   debuggable(false)
    ,   maxAllocatedMemory(0)
{
}

void BaseTest::LoadResources()
{
    const Size2i& size = DAVA::VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();

    scene = new Scene();

    Rect rect;
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.dx = size.dx;
    rect.dy = size.dy;

    sceneView = new UI3DView(rect, true);
    sceneView->SetScene(scene);

    AddControl(sceneView);
}

void BaseTest::UnloadResources()
{
    SafeRelease(scene);
}

size_t BaseTest::GetAllocatedMemory()
{
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    MemoryManager* mm = MemoryManager::Instance();
    size_t bufSize = mm->CalcStatSize();
    uint8* buf = new uint8[bufSize];

    MMStat* stat = reinterpret_cast<MMStat*>(buf);
    mm->GetStat(stat);

    size_t total = 0;
    for (uint32 ipool = 0; ipool < stat->allocPoolCount; ++ipool)
    {
        total += stat->poolStat[ipool].allocByApp;
    }

    delete[] buf;
    return total;
#else
    return 0;
#endif

}
void BaseTest::OnStart()
{
    Logger::Info(TeamcityTestsOutput::FormatTestStarted(testName).c_str());
}

void BaseTest::OnFinish()
{
    elapsedTime = SystemTimer::Instance()->FrameStampTimeMS() - startTime;

    float32 minDelta = FLT_MAX;
    float32 maxDelta = FLT_MIN;
    float32 averageDelta = 0.0f;

    float32 testTime = 0.0f;
    float32 elapsedTime = 0.0f;

    size_t framesCount = GetFramesInfo().size();

    Logger::Info(("TestName:" + testName).c_str());
    
    for (const BaseTest::FrameInfo& frameInfo : GetFramesInfo())
    {
        if (frameInfo.delta > maxDelta)
        {
            maxDelta = frameInfo.delta;
        }
        if (frameInfo.delta < minDelta)
        {
            minDelta = frameInfo.delta;
        }

        averageDelta += frameInfo.delta;

        Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::FRAME_DELTA,
            DAVA::Format("%f", frameInfo.delta)).c_str());
    }

    averageDelta /= framesCount;

    testTime = GetTestTime();
    elapsedTime = GetElapsedTime() / 1000.0f;

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::MIN_DELTA,
        DAVA::Format("%f", minDelta)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::MAX_DELTA,
        DAVA::Format("%f", maxDelta)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::AVERAGE_DELTA,
        DAVA::Format("%f", averageDelta)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::MAX_FPS,
        DAVA::Format("%f", 1.0f / minDelta)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::MIN_FPS,
        DAVA::Format("%f", 1.0f / maxDelta)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::AVERAGE_FPS,
        DAVA::Format("%f", 1.0f / averageDelta)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::TEST_TIME,
        DAVA::Format("%f", testTime)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::TIME_ELAPSED,
        DAVA::Format("%f", elapsedTime)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
        TeamcityTestsOutput::MAX_MEM_USAGE,
        DAVA::Format("%d", maxAllocatedMemory)).c_str());

    Logger::Info(TeamcityTestsOutput::FormatTestFinished(testName).c_str());
}

void BaseTest::SystemUpdate(float32 timeElapsed)
{
    size_t allocatedMem = GetAllocatedMemory();
    if (allocatedMem > maxAllocatedMemory)
    {
        maxAllocatedMemory = allocatedMem;
    }

    bool frameForDebug = GetFrameNumber() >= (GetDebugFrame() + BaseTest::FRAME_OFFSET);
    bool greaterMaxDelta = maxDelta > 0.0f && maxDelta <= timeElapsed; 
    float32 delta = 0.0f;

    if (IsDebuggable() && (frameForDebug || greaterMaxDelta))
    {
        if (greaterMaxDelta)
        {
            Logger::Info(DAVA::Format("Time delta: %f \nMaxDelta: %f \nFrame : %d", timeElapsed, maxDelta, frameNumber).c_str());
        }
        if (GetFrameNumber() == (GetDebugFrame() + BaseTest::FRAME_OFFSET))
        {
            Logger::Info(DAVA::Format("Frame for debug: %d", frameNumber - BaseTest::FRAME_OFFSET).c_str());
        }
    }
    else if (frameNumber > FRAME_OFFSET)
    {
        delta = targetFrameDelta > 0 ? targetFrameDelta : timeElapsed;

        frames.push_back(FrameInfo(timeElapsed));
        testTime += timeElapsed;

        PerformTestLogic(delta);
    }
   
    BaseScreen::SystemUpdate(delta);
}

void BaseTest::BeginFrame()
{
    if (frameNumber > (FRAME_OFFSET - 1) && startTime == 0)
    {
        startTime = SystemTimer::Instance()->FrameStampTimeMS();
    }
}

void BaseTest::EndFrame()
{
    frameNumber++;
}