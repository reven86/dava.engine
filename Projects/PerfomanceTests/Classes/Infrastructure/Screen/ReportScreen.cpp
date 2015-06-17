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

#include "ReportScreen.h"

const String ReportScreen::MIN_DELTA = "MinDeltaValue";
const String ReportScreen::MAX_DELTA = "MaxDeltaValue";
const String ReportScreen::AVERAGE_DELTA = "AverageDeltaValue";
const String ReportScreen::TEST_TIME = "TestTimeValue";
const String ReportScreen::ELAPSED_TIME = "ElapsedTimeValue";
const String ReportScreen::FRAMES_RENDERED = "FramesRenderedValue";

ReportScreen::ReportScreen(const Vector<BaseTest*>& _testChain)
    :   testChain(_testChain)
{
}

bool ReportScreen::IsFinished() const
{
    return false;
}

void ReportScreen::LoadResources()
{
    CreateReportScreen();
}

void ReportScreen::UnloadResources()
{
}

void ReportScreen::CreateReportScreen()
{
    UIControl* reportItem = new UIControl();

    UIYamlLoader::LoadFonts("~res:/UI/Fonts/fonts.yaml");
    UIYamlLoader::Load(reportItem, ControlHelpers::GetPathToUIYaml("ReportItem.yaml"));
    
    uint32 offsetY = 150;
    uint32 testNumber = 0;
    
    for (BaseTest* test : testChain)
    {
        if (test->IsFinished())
        {
            Vector<BaseTest::FrameInfo> frameInfoList = test->GetFramesInfo();
            
            float32 minDelta = FLT_MAX;
            float32 maxDelta = FLT_MIN;
            float32 averageDelta = 0.0f;
            
            float32 testTime = 0.0f;
            float32 elapsedTime = 0.0f;
            
            uint32 framesCount = test->GetFramesInfo().size();
            
            for (BaseTest::FrameInfo frameInfo : frameInfoList)
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
            }

            averageDelta /= framesCount;

            testTime = test->GetTestTime();
            elapsedTime = test->GetElapsedTime() / 1000.0f;

            UIControl* reportItemCopy = reportItem->Clone();
            reportItemCopy->SetPosition(Vector2(0.0f, 0.0f + testNumber * offsetY));

            UIStaticText* minDeltaText = static_cast<UIStaticText*>(reportItemCopy->FindByName(MIN_DELTA));
            minDeltaText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", minDelta)));

            UIStaticText* maxDeltaText = static_cast<UIStaticText*>(reportItemCopy->FindByName(MAX_DELTA));
            maxDeltaText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", maxDelta)));

            UIStaticText* averageDeltaText = static_cast<UIStaticText*>(reportItemCopy->FindByName(AVERAGE_DELTA));
            averageDeltaText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", averageDelta)));

            UIStaticText* testTimeText = static_cast<UIStaticText*>(reportItemCopy->FindByName(TEST_TIME));
            testTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", testTime)));

            UIStaticText* elapsedTimeText = static_cast<UIStaticText*>(reportItemCopy->FindByName(ELAPSED_TIME));
            elapsedTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", elapsedTime)));

            UIStaticText* framesRenderedText = static_cast<UIStaticText*>(reportItemCopy->FindByName(FRAMES_RENDERED));
            framesRenderedText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%d", framesCount)));

            AddControl(reportItemCopy);
            
            testNumber++;
        }
    }

    SafeRelease(reportItem);
}
