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

const uint32 BaseTest::FRAME_OFFSET = 5;

BaseTest::BaseTest(const String& _testName, const TestParams& _testParams)
    :   testName(_testName)
    ,   testParams(_testParams)
    ,   frameNumber(1)
    ,   startTime(0)
    ,   overallTestTime(0.0f)
    ,   minDelta(FLT_MAX)
    ,   maxDelta(FLT_MIN)
    ,   currentFrameDelta(0.0f)
    ,   uiRoot(new DAVA::UIControl())
    ,   maxAllocatedMemory(0)
{
    sceneName = testName + ": " + GetParams().sceneName;
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
    
    CreateUI();
}

void BaseTest::UnloadResources()
{
    SafeRelease(scene);
    SafeRelease(sceneView);
}

void BaseTest::CreateUI()
{
    ScopedPtr<DAVA::UIControl> reportItem(new DAVA::UIControl());
    
    UIYamlLoader::LoadFonts("~res:/UI/Fonts/fonts.yaml");
    UIYamlLoader::Load(reportItem, ControlHelpers::GetPathToUIYaml("ReportItem.yaml"));
    
    uiRoot->SetPosition(Vector2(0.0f, 0.0f));
    reportItem->SetPosition(Vector2(0.0f, 0.0f));
    
    testNameText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::TEST_NAME_PATH);
    testNameText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%s", GetSceneName().c_str())));
    
    UIStaticText* fieldMinFpsText = reportItem->FindByPath<UIStaticText*>("MinDelta/MinDeltaText");
    fieldMinFpsText->SetText(UTF8Utils::EncodeToWideString("Max FPS"));
    
    maxFPSText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::MIN_DELTA_PATH);
    maxFPSText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 0.0f)));
    
    UIStaticText* fieldMaxFpsText = reportItem->FindByPath<UIStaticText*>("MaxDelta/MaxDeltaText");
    fieldMaxFpsText->SetText(UTF8Utils::EncodeToWideString("Min FPS"));
    
    minFPSText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::MAX_DELTA_PATH);
    minFPSText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 0.0f)));
    
    UIStaticText* fieldFpsText = reportItem->FindByPath<UIStaticText*>("AverageDelta/AverageDeltaText");
    fieldFpsText->SetText(UTF8Utils::EncodeToWideString("FPS"));
    
    fpsText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::AVERAGE_DELTA_PATH);
    fpsText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 0.0f)));
    
    testTimeText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::TEST_TIME_PATH);
    testTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 0.0f)));
    
    elapsedTimeText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::ELAPSED_TIME_PATH);
    elapsedTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 0.0f)));
    
    framesRenderedText = reportItem->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::FRAMES_RENDERED_PATH);
    framesRenderedText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%d", 0)));
    
    AddControl(uiRoot);
    uiRoot->AddControl(reportItem);
}

void BaseTest::UpdateUI()
{
    float32 fps = currentFrameDelta > 0.001f ? 1.0f / currentFrameDelta : 0.0f;
    
    maxFPSText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 1.0f / minDelta)));
    minFPSText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", 1.0f / maxDelta)));
    fpsText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", fps)));
    testTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", overallTestTime)));
    elapsedTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", elapsedTime / 1000.0f)));
    framesRenderedText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%d", GetTestFrameNumber())));
}

size_t BaseTest::GetAllocatedMemory()
{
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
    return MemoryManager::Instance()->GetTrackedMemoryUsage();
#else
    return 0;
#endif

}
void BaseTest::OnStart()
{
    Logger::Info(TeamcityTestsOutput::FormatTestStarted(GetSceneName()).c_str());
}

void BaseTest::OnFinish()
{
    PrintStatistic(GetFramesInfo());

    Logger::Info(TeamcityTestsOutput::FormatTestFinished(GetSceneName()).c_str());
}

void BaseTest::PrintStatistic(const Vector<FrameInfo>& frames)
{
    size_t framesCount = GetFramesInfo().size();
    
    for (const auto& frameInfo : frames)
    {
        Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::FRAME_DELTA,
            DAVA::Format("%f", frameInfo.delta)).c_str());
    }
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::MIN_DELTA,
            DAVA::Format("%f", minDelta)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::MAX_DELTA,
            DAVA::Format("%f", maxDelta)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::AVERAGE_DELTA,
            DAVA::Format("%f", overallTestTime / framesCount)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::MAX_FPS,
            DAVA::Format("%f", 1.0f / minDelta)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::MIN_FPS,
            DAVA::Format("%f", 1.0f / maxDelta)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::AVERAGE_FPS,
            DAVA::Format("%f", 1.0f / (overallTestTime / framesCount))).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::TEST_TIME,
            DAVA::Format("%f", overallTestTime)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::TIME_ELAPSED,
            DAVA::Format("%f", elapsedTime / 1000.0f)).c_str());
    
    Logger::Info(TeamcityTestsOutput::FormatBuildStatistic(
            TeamcityTestsOutput::MAX_MEM_USAGE,
            DAVA::Format("%d", maxAllocatedMemory)).c_str());
}

void BaseTest::SystemUpdate(float32 timeElapsed)
{
    size_t allocatedMem = GetAllocatedMemory();
    if (allocatedMem > maxAllocatedMemory)
    {
        maxAllocatedMemory = allocatedMem;
    }

    bool isFrameForDebug = testParams.frameForDebug > 0 && GetTestFrameNumber() >= testParams.frameForDebug;
    bool isGreaterMaxDelta = testParams.maxDelta > 0.001f && timeElapsed >= testParams.maxDelta;
    
    float32 delta = 0.0f;
    float32 currentTimeMs = overallTestTime * 1000;
    
    if (frameNumber > FRAME_OFFSET)
    {
        if (currentTimeMs >= testParams.startTime && currentTimeMs <= testParams.endTime)
        {
            if (isGreaterMaxDelta)
            {
                Logger::Info(DAVA::Format("Time delta: %f \nMaxDelta: %f \nFrame : %d", timeElapsed, testParams.maxDelta, frameNumber).c_str());
            }
            if (isFrameForDebug)
            {
                Logger::Info(DAVA::Format("Frame for debug: %d", frameNumber - BaseTest::FRAME_OFFSET).c_str());
            }
            
            frames.push_back(FrameInfo(timeElapsed));
        }
        
        if (!isFrameForDebug && !isGreaterMaxDelta)
        {
            delta = testParams.targetFrameDelta > 0.001f ? testParams.targetFrameDelta : timeElapsed;
        }
        
        overallTestTime += delta;
        currentFrameDelta = delta;
        
        if(delta < minDelta)
        {
            minDelta = delta;
        }
        if(delta > maxDelta)
        {
            maxDelta = delta;
        }
        
        if(IsUIVisible())
        {
            UpdateUI();
        }
        
        PerformTestLogic(delta);
    }
    
    BaseScreen::SystemUpdate(delta);
}

void BaseTest::BeginFrame()
{
    if (0 == startTime && frameNumber >= FRAME_OFFSET)
    {
        startTime = SystemTimer::Instance()->FrameStampTimeMS();
    }
}

void BaseTest::EndFrame()
{
    frameNumber++;
    
    elapsedTime = SystemTimer::Instance()->FrameStampTimeMS() - startTime;
}

bool BaseTest::IsFinished() const
{
    if (testParams.targetFramesCount > 0)
    {
        if (GetTestFrameNumber() > testParams.targetFramesCount)
        {
            return true;
        }
    }
    else if (testParams.targetTime > 0 && (overallTestTime * 1000) >= testParams.targetTime)
    {
        return true;
    }
    
    return false;
}

const String& BaseTest::GetSceneName() const
{
    return sceneName;
}