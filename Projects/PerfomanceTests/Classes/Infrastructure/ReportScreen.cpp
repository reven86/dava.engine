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


ReportScreen::ReportScreen(const Vector<BaseTest*>& _testChain) : 
testChain(_testChain)

{
}


ReportScreen::~ReportScreen()
{
}

void ReportScreen::OnStart(HashMap<String, BaseObject*>& params)
{
	CreateReportScreen();
}

void ReportScreen::OnFinish(HashMap<String, BaseObject*>& params)
{

}

void ReportScreen::BeginFrame()
{
	RenderSystem2D::Instance()->Reset();
	RenderManager::Instance()->BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
}

void ReportScreen::EndFrame()
{
	RenderManager::Instance()->EndFrame();
	RenderManager::Instance()->ProcessStats();
}

void ReportScreen::Update(float32 timeElapsed)
{
	UIControlSystem::Instance()->Update();
}

void ReportScreen::Draw()
{
	UIControlSystem::Instance()->Draw();
}

bool ReportScreen::IsFinished() const
{
	return false;
}

void ReportScreen::CreateReportScreen()
{
	UIScreen* reportScreen = new UIScreen();

	UIScreenManager::Instance()->RegisterScreen(0, reportScreen);
	UIScreenManager::Instance()->SetFirst(0);

	Font* font = FTFont::Create("./Data/Fonts/korinna.ttf");
	uint32 offsetY = 150;
	uint32 testNumber = 0;

	for each (BaseTest* test in testChain)
	{
		if (test->IsPerformed())
		{
			List<BaseTest::FrameInfo> frameInfoList = test->GetFramesInfo();

			float32 minDelta = FLT_MAX;
			float32 maxDelta = FLT_MIN;
			float32 averageDelta = 0.0f;

			float32 testTime = 0.0f;
			float32 elapsedTime = 0.0f;

			uint32 framesCount = test->GetFramesInfo().size();

			for each (BaseTest::FrameInfo frameInfo in frameInfoList)
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

			UIStaticText* testNameText = new UIStaticText();
			testNameText->SetFont(font);
			testNameText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			testNameText->SetPosition(Vector2(10.0f, 10.0f + offsetY * testNumber));
			testNameText->SetText(UTF8Utils::EncodeToWideString(test->GetName() + ":"));
			testNameText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			testNameText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* upDelimiter = new UIStaticText();
			upDelimiter->SetFont(font);
			upDelimiter->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			upDelimiter->SetPosition(Vector2(5.0f, 25.0f + offsetY * testNumber));
			upDelimiter->SetText(UTF8Utils::EncodeToWideString("======================="));
			upDelimiter->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			upDelimiter->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* minDeltaText = new UIStaticText();
			minDeltaText->SetFont(font);
			minDeltaText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			minDeltaText->SetPosition(Vector2(10.0f, 40.0f + offsetY * testNumber));
			minDeltaText->SetText(UTF8Utils::EncodeToWideString("Min delta: "));
			minDeltaText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			minDeltaText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* minDeltaValue = new UIStaticText();
			minDeltaValue->SetFont(font);
			minDeltaValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			minDeltaValue->SetPosition(Vector2(100.0f, 40.0f + offsetY * testNumber));
			minDeltaValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(minDelta)));
			minDeltaValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			minDeltaValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* maxDeltaText = new UIStaticText();
			maxDeltaText->SetFont(font);
			maxDeltaText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			maxDeltaText->SetPosition(Vector2(10.0f, 55.0f + offsetY * testNumber));
			maxDeltaText->SetText(UTF8Utils::EncodeToWideString("Max delta: "));
			maxDeltaText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			maxDeltaText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* maxDeltaValue = new UIStaticText();
			maxDeltaValue->SetFont(font);
			maxDeltaValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			maxDeltaValue->SetPosition(Vector2(100.0f, 55.0f + offsetY * testNumber));
			maxDeltaValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(maxDelta)));
			maxDeltaValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			maxDeltaValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* averageDeltaText = new UIStaticText();
			averageDeltaText->SetFont(font);
			averageDeltaText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			averageDeltaText->SetPosition(Vector2(10.0f, 70.0f + offsetY * testNumber));
			averageDeltaText->SetText(UTF8Utils::EncodeToWideString("Average delta: "));
			averageDeltaText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			averageDeltaText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* averageDeltaValue = new UIStaticText();
			averageDeltaValue->SetFont(font);
			averageDeltaValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			averageDeltaValue->SetPosition(Vector2(100.0f, 70.0f + offsetY * testNumber));
			averageDeltaValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(averageDelta)));
			averageDeltaValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			averageDeltaValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* testTimeText = new UIStaticText();
			testTimeText->SetFont(font);
			testTimeText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			testTimeText->SetPosition(Vector2(10.0f, 85.0f + offsetY * testNumber));
			testTimeText->SetText(UTF8Utils::EncodeToWideString("Test time: "));
			testTimeText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			testTimeText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* testTimeValue = new UIStaticText();
			testTimeValue->SetFont(font);
			testTimeValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			testTimeValue->SetPosition(Vector2(100.0f, 85.0f + offsetY * testNumber));
			testTimeValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(testTime)));
			testTimeValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			testTimeValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* elapsedTimeText = new UIStaticText();
			elapsedTimeText->SetFont(font);
			elapsedTimeText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			elapsedTimeText->SetPosition(Vector2(10.0f, 100.0f + offsetY * testNumber));
			elapsedTimeText->SetText(UTF8Utils::EncodeToWideString("Elapsed time: "));
			elapsedTimeText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			elapsedTimeText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* elapsedTimeValue = new UIStaticText();
			elapsedTimeValue->SetFont(font);
			elapsedTimeValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			elapsedTimeValue->SetPosition(Vector2(100.0f, 100.0f + offsetY * testNumber));
			elapsedTimeValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(elapsedTime)));
			elapsedTimeValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			elapsedTimeValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* framesText = new UIStaticText();
			framesText->SetFont(font);
			framesText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			framesText->SetPosition(Vector2(10.0f, 115.0f + offsetY * testNumber));
			framesText->SetText(UTF8Utils::EncodeToWideString("Frames rendered: "));
			framesText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			framesText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* framesValue = new UIStaticText();
			framesValue->SetFont(font);
			framesValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			framesValue->SetPosition(Vector2(100.0f, 115.0f + offsetY * testNumber));
			framesValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(framesCount)));
			framesValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			framesValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* downDelimiter = new UIStaticText();
			downDelimiter->SetFont(font);
			downDelimiter->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			downDelimiter->SetPosition(Vector2(5.0f, 130.0f + offsetY * testNumber));
			downDelimiter->SetText(UTF8Utils::EncodeToWideString("======================="));
			downDelimiter->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			downDelimiter->SetSize(Vector2(150.0f, 10.0f));

			reportScreen->AddControl(testNameText);
			reportScreen->AddControl(upDelimiter);
			reportScreen->AddControl(minDeltaText);
			reportScreen->AddControl(minDeltaValue);
			reportScreen->AddControl(maxDeltaText);
			reportScreen->AddControl(maxDeltaValue);
			reportScreen->AddControl(averageDeltaText);
			reportScreen->AddControl(averageDeltaValue);
			reportScreen->AddControl(testTimeText);
			reportScreen->AddControl(testTimeValue);
			reportScreen->AddControl(elapsedTimeText);
			reportScreen->AddControl(elapsedTimeValue);
			reportScreen->AddControl(framesText);
			reportScreen->AddControl(framesValue);
			reportScreen->AddControl(downDelimiter);

			testNumber++;
		}
	}
}
