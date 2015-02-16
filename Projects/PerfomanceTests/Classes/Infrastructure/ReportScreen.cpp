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


ReportScreen::ReportScreen(Vector<BaseTest*>& _testsChain) : testsChain(_testsChain)
{
}


ReportScreen::~ReportScreen()
{
}

void ReportScreen::OnStart()
{

}

void ReportScreen::OnFinish()
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
	UIScreen* pReportScreen = new UIScreen();

	UIScreenManager::Instance()->RegisterScreen(0, pReportScreen);
	UIScreenManager::Instance()->SetFirst(0);

	Font* pFont = FTFont::Create("./Data/Fonts/korinna.ttf");
	uint32 offsetY = 150;
	uint32 testNumber = 0;

	for each (BaseTest* pTest in testsChain)
	{
		if (pTest->IsFinished())
		{
			List<BaseTest::FrameInfo> frameInfoList = pTest->GetFramesInfo();

			float32 minDelta = FLT_MAX;
			float32 maxDelta = FLT_MIN;
			float32 averageDelta = 0.0f;

			float32 testTime = 0.0f;
			float32 elapsedTime = 0.0f;

			uint32 framesCount = pTest->GetFramesInfo().size();

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

			testTime = pTest->GetTestTime();
			elapsedTime = pTest->GetElapsedTime() / 1000.0f;

			UIStaticText* pTestNameText = new UIStaticText();
			pTestNameText->SetFont(pFont);
			pTestNameText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pTestNameText->SetPosition(Vector2(10.0f, 10.0f + offsetY * testNumber));
			pTestNameText->SetText(UTF8Utils::EncodeToWideString(pTest->GetName() + ":"));
			pTestNameText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pTestNameText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pUpDelimiter = new UIStaticText();
			pUpDelimiter->SetFont(pFont);
			pUpDelimiter->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pUpDelimiter->SetPosition(Vector2(5.0f, 25.0f + offsetY * testNumber));
			pUpDelimiter->SetText(UTF8Utils::EncodeToWideString("======================="));
			pUpDelimiter->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pUpDelimiter->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pMinDeltaText = new UIStaticText();
			pMinDeltaText->SetFont(pFont);
			pMinDeltaText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pMinDeltaText->SetPosition(Vector2(10.0f, 40.0f + offsetY * testNumber));
			pMinDeltaText->SetText(UTF8Utils::EncodeToWideString("Min delta: "));
			pMinDeltaText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pMinDeltaText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pMinDeltaValue = new UIStaticText();
			pMinDeltaValue->SetFont(pFont);
			pMinDeltaValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			pMinDeltaValue->SetPosition(Vector2(100.0f, 40.0f + offsetY * testNumber));
			pMinDeltaValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(minDelta)));
			pMinDeltaValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pMinDeltaValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* pMaxDeltaText = new UIStaticText();
			pMaxDeltaText->SetFont(pFont);
			pMaxDeltaText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pMaxDeltaText->SetPosition(Vector2(10.0f, 55.0f + offsetY * testNumber));
			pMaxDeltaText->SetText(UTF8Utils::EncodeToWideString("Max delta: "));
			pMaxDeltaText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pMaxDeltaText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pMaxDeltaValue = new UIStaticText();
			pMaxDeltaValue->SetFont(pFont);
			pMaxDeltaValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			pMaxDeltaValue->SetPosition(Vector2(100.0f, 55.0f + offsetY * testNumber));
			pMaxDeltaValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(maxDelta)));
			pMaxDeltaValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pMaxDeltaValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* pAverageDeltaText = new UIStaticText();
			pAverageDeltaText->SetFont(pFont);
			pAverageDeltaText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pAverageDeltaText->SetPosition(Vector2(10.0f, 70.0f + offsetY * testNumber));
			pAverageDeltaText->SetText(UTF8Utils::EncodeToWideString("Average delta: "));
			pAverageDeltaText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pAverageDeltaText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pAverageDeltaValue = new UIStaticText();
			pAverageDeltaValue->SetFont(pFont);
			pAverageDeltaValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			pAverageDeltaValue->SetPosition(Vector2(100.0f, 70.0f + offsetY * testNumber));
			pAverageDeltaValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(averageDelta)));
			pAverageDeltaValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pAverageDeltaValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* pTestTimeText = new UIStaticText();
			pTestTimeText->SetFont(pFont);
			pTestTimeText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pTestTimeText->SetPosition(Vector2(10.0f, 85.0f + offsetY * testNumber));
			pTestTimeText->SetText(UTF8Utils::EncodeToWideString("Test time: "));
			pTestTimeText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pTestTimeText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pTestTimeValue = new UIStaticText();
			pTestTimeValue->SetFont(pFont);
			pTestTimeValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			pTestTimeValue->SetPosition(Vector2(100.0f, 85.0f + offsetY * testNumber));
			pTestTimeValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(testTime)));
			pTestTimeValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pTestTimeValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* pElapsedTimeText = new UIStaticText();
			pElapsedTimeText->SetFont(pFont);
			pElapsedTimeText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pElapsedTimeText->SetPosition(Vector2(10.0f, 100.0f + offsetY * testNumber));
			pElapsedTimeText->SetText(UTF8Utils::EncodeToWideString("Elapsed time: "));
			pElapsedTimeText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pElapsedTimeText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pElapsedTimeValue = new UIStaticText();
			pElapsedTimeValue->SetFont(pFont);
			pElapsedTimeValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			pElapsedTimeValue->SetPosition(Vector2(100.0f, 100.0f + offsetY * testNumber));
			pElapsedTimeValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(elapsedTime)));
			pElapsedTimeValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pElapsedTimeValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* pFramesText = new UIStaticText();
			pFramesText->SetFont(pFont);
			pFramesText->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pFramesText->SetPosition(Vector2(10.0f, 115.0f + offsetY * testNumber));
			pFramesText->SetText(UTF8Utils::EncodeToWideString("Frames rendered: "));
			pFramesText->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pFramesText->SetSize(Vector2(150.0f, 10.0f));

			UIStaticText* pFramesValue = new UIStaticText();
			pFramesValue->SetFont(pFont);
			pFramesValue->SetTextAlign(ALIGN_RIGHT | ALIGN_VCENTER);
			pFramesValue->SetPosition(Vector2(100.0f, 115.0f + offsetY * testNumber));
			pFramesValue->SetText(UTF8Utils::EncodeToWideString(std::to_string(framesCount)));
			pFramesValue->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pFramesValue->SetSize(Vector2(100.0f, 10.0f));

			UIStaticText* pDownDelimiter = new UIStaticText();
			pDownDelimiter->SetFont(pFont);
			pDownDelimiter->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
			pDownDelimiter->SetPosition(Vector2(5.0f, 130.0f + offsetY * testNumber));
			pDownDelimiter->SetText(UTF8Utils::EncodeToWideString("======================="));
			pDownDelimiter->SetTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
			pDownDelimiter->SetSize(Vector2(150.0f, 10.0f));

			pReportScreen->AddControl(pTestNameText);
			pReportScreen->AddControl(pUpDelimiter);
			pReportScreen->AddControl(pMinDeltaText);
			pReportScreen->AddControl(pMinDeltaValue);
			pReportScreen->AddControl(pMaxDeltaText);
			pReportScreen->AddControl(pMaxDeltaValue);
			pReportScreen->AddControl(pAverageDeltaText);
			pReportScreen->AddControl(pAverageDeltaValue);
			pReportScreen->AddControl(pTestTimeText);
			pReportScreen->AddControl(pTestTimeValue);
			pReportScreen->AddControl(pElapsedTimeText);
			pReportScreen->AddControl(pElapsedTimeValue);
			pReportScreen->AddControl(pFramesText);
			pReportScreen->AddControl(pFramesValue);
			pReportScreen->AddControl(pDownDelimiter);

			testNumber++;
		}
	}
}
