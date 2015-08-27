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

bool ReportScreen::IsFinished() const
{
    return false;
}

void ReportScreen::LoadResources()
{
    BaseScreen::LoadResources();

    CreateReportScreen();
}

void ReportScreen::CreateReportScreen()
{
    ScopedPtr<UIControl> reportItem(new UIControl());

    UIYamlLoader::LoadFonts("~res:/UI/Fonts/fonts.yaml");
    UIYamlLoader::Load(reportItem, ControlHelpers::GetPathToUIYaml("ReportItem.yaml"));
    
    uint32 offsetY = 150;
    uint32 testNumber = 0;
    
    for (auto* test : testChain)
    {
        if (test->IsFinished())
        {
            const auto& framesInfo = test->GetFramesInfo();
            
            float32 minDelta = FLT_MAX;
            float32 maxDelta = FLT_MIN;
            float32 averageDelta = 0.0f;
            
            float32 testTime = 0.0f;
            float32 elapsedTime = 0.0f;
            
            uint32 framesCount = framesInfo.size();
            
            for (const auto& frameInfo : framesInfo)
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

            testTime = test->GetOverallTestTime();
            elapsedTime = test->GetElapsedTime() / 1000.0f;

            ScopedPtr<UIControl> reportItemCopy(reportItem->Clone());
            reportItemCopy->SetPosition(Vector2(0.0f, 0.0f + testNumber * offsetY));
            
            UIStaticText* testName = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::TEST_NAME_PATH);
            testName->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%s", test->GetSceneName().c_str())));

            UIStaticText* minDeltaText = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::MIN_DELTA_PATH);
            minDeltaText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", minDelta)));

            UIStaticText* maxDeltaText = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::MAX_DELTA_PATH);
            maxDeltaText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", maxDelta)));

            UIStaticText* averageDeltaText = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::AVERAGE_DELTA_PATH);
            averageDeltaText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", averageDelta)));

            UIStaticText* testTimeText = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::TEST_TIME_PATH);
            testTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", testTime)));

            UIStaticText* elapsedTimeText = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::ELAPSED_TIME_PATH);
            elapsedTimeText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%f", elapsedTime)));

            UIStaticText* framesRenderedText = reportItemCopy->FindByPath<UIStaticText*>(ControlHelpers::ReportItem::FRAMES_RENDERED_PATH);
            framesRenderedText->SetText(UTF8Utils::EncodeToWideString(DAVA::Format("%d", framesCount)));

            AddControl(reportItemCopy);
            
            testNumber++;
        }
    }
}
