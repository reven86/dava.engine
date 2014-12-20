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

#include "BiDiTest.h"

static const float BIDI_TEST_AUTO_CLOSE_TIME = 30.0f;

static const sBiDiTestData TEST_DATA[] =
{
	// Input: Text, align, useRtl; Test output: visualAlign, isRtl
	
	// English text, LTR
	sBiDiTestData(L"English text", ALIGN_LEFT,    true,  ALIGN_LEFT,    false),
	sBiDiTestData(L"English text", ALIGN_HCENTER, true,  ALIGN_HCENTER, false),
	sBiDiTestData(L"English text", ALIGN_RIGHT,   true,  ALIGN_RIGHT,   false),
	sBiDiTestData(L"English text", ALIGN_LEFT,    false, ALIGN_LEFT,    false),
	sBiDiTestData(L"English text", ALIGN_HCENTER, false, ALIGN_HCENTER, false),
	sBiDiTestData(L"English text", ALIGN_RIGHT,   false, ALIGN_RIGHT,   false),
	
	// Arabic text, RTL
	sBiDiTestData(L"النص العربي", ALIGN_LEFT,    true,  ALIGN_RIGHT,   true),
	sBiDiTestData(L"النص العربي", ALIGN_HCENTER, true,  ALIGN_HCENTER, true),
	sBiDiTestData(L"النص العربي", ALIGN_RIGHT,   true,  ALIGN_LEFT,    true),
	sBiDiTestData(L"النص العربي", ALIGN_LEFT,    false, ALIGN_LEFT,    true),
	sBiDiTestData(L"النص العربي", ALIGN_HCENTER, false, ALIGN_HCENTER, true),
	sBiDiTestData(L"النص العربي", ALIGN_RIGHT,   false, ALIGN_RIGHT,   true),
	
	// Mixed english and arabic text, english symbols firts, LTR
	sBiDiTestData(L"Mixed text النص العربي", ALIGN_LEFT,    true,  ALIGN_LEFT,    false),
	sBiDiTestData(L"Mixed text النص العربي", ALIGN_HCENTER, true,  ALIGN_HCENTER, false),
	sBiDiTestData(L"Mixed text النص العربي", ALIGN_RIGHT,   true,  ALIGN_RIGHT,   false),
	sBiDiTestData(L"Mixed text النص العربي", ALIGN_LEFT,    false, ALIGN_LEFT,    false),
	sBiDiTestData(L"Mixed text النص العربي", ALIGN_HCENTER, false, ALIGN_HCENTER, false),
	sBiDiTestData(L"Mixed text النص العربي", ALIGN_RIGHT,   false, ALIGN_RIGHT,   false),
	
	// Mised arabic and englich text, arabic symbols first, RTL
	sBiDiTestData(L"النص العربي mixed text", ALIGN_LEFT,    true,  ALIGN_RIGHT,   true),
	sBiDiTestData(L"النص العربي mixed text", ALIGN_HCENTER, true,  ALIGN_HCENTER, true),
	sBiDiTestData(L"النص العربي mixed text", ALIGN_RIGHT,   true,  ALIGN_LEFT,    true),
	sBiDiTestData(L"النص العربي mixed text", ALIGN_LEFT,    false, ALIGN_LEFT,    true),
	sBiDiTestData(L"النص العربي mixed text", ALIGN_HCENTER, false, ALIGN_HCENTER, true),
	sBiDiTestData(L"النص العربي mixed text", ALIGN_RIGHT,   false, ALIGN_RIGHT,   true),
};

BiDiTest::BiDiTest()
    : TestTemplate<BiDiTest>("BiDiTest")
{
    textField = NULL;
	staticText = NULL;
	testButton = NULL;
	
	onScreenTime = 0.0f;
	testFinished = false;
	manualStarted = false;
	
	uint32 testDataSize = sizeof(TEST_DATA) / sizeof(sBiDiTestData);
	
	for (uint32 i = 0; i < testDataSize; ++i)
	{
		const sBiDiTestData& d = TEST_DATA[i];
		RegisterFunction(this, &BiDiTest::TestFunction, Format("BiDiTest"), new sBiDiTestData(d.text, d.align | ALIGN_VCENTER, d.useRtl, d.visualAlign | ALIGN_VCENTER, d.isRtl));
	}
	
	// Functions for manual testing - just wait BIDI_TEST_AUTO_CLOSE_TIME seconds
#if 0
	RegisterFunction(this, &BiDiTest::ManualTestStartFunction, Format("BiDiTestManual"), NULL);
	RegisterFunction(this, &BiDiTest::ManualTestProcessFunction, Format("BiDiTestManual"), NULL);
#endif
	
}

void BiDiTest::LoadResources()
{
    GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Font *font = FTFont::Create("~res:/Fonts/arialuni.ttf");
    DVASSERT(font);
	font->SetSize(20);
	
    staticText = new UIStaticText(Rect(10, 10, 300, 50));
	staticText->SetFont(font);
    staticText->SetTextColor(Color::White);
	staticText->SetTextAlign(ALIGN_VCENTER | ALIGN_LEFT);
	staticText->SetText(L"textField");
	staticText->SetDebugDraw(true);
	AddControl(staticText);

    textField = new UITextField(Rect(10, 70, 300, 50));
	textField->SetFont(font);
    textField->SetTextColor(Color::White);
    textField->SetTextAlign(ALIGN_VCENTER | ALIGN_LEFT);
    textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	textField->SetDelegate(this);
	AddControl(textField);

	modeButton = new UIButton(Rect(320, 10, 200, 50));
	modeButton->SetStateFont(0xFF, font);
	modeButton->SetStateFontColor(0xFF, Color::White);
	modeButton->SetStateText(0xFF, L"Align: Left");
	modeButton->SetDebugDraw(true);
	modeButton->AddEvent(EVENT_TOUCH_UP_INSIDE, Message(this, &BiDiTest::ButtonPressed));
    AddControl(modeButton);
	
	rtlButton = new UIButton(Rect(320, 70, 200, 50));
	rtlButton->SetStateFont(0xFF, font);
	rtlButton->SetStateFontColor(0xFF, Color::White);
	rtlButton->SetStateText(0xFF, L"RTL Align: Enabled");
	rtlButton->SetDebugDraw(true);
	rtlButton->AddEvent(EVENT_TOUCH_UP_INSIDE, Message(this, &BiDiTest::ButtonPressed));
    AddControl(rtlButton);
	
	autoStaticText = new UIStaticText(Rect(10, 140, 510, 50));
	autoStaticText->SetFont(font);
    autoStaticText->SetTextColor(Color::White);
	autoStaticText->SetTextAlign(ALIGN_VCENTER | ALIGN_LEFT);
	autoStaticText->SetText(L"autoTestTextField");
	autoStaticText->SetDebugDraw(true);
	AddControl(autoStaticText);
	
    testButton = new UIButton(Rect(10, 210, 510, 50));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateFontColor(0xFF, Color::White);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(EVENT_TOUCH_UP_INSIDE, Message(this, &BiDiTest::ButtonPressed));
    AddControl(testButton);

    SafeRelease(font);
}

void BiDiTest::UnloadResources()
{
    RemoveAllControls();

	SafeRelease(testButton);
	SafeRelease(textField);
	SafeRelease(staticText);
}

void BiDiTest::Update(float32 timeElapsed)
{
	if(manualStarted)
	{
		onScreenTime += timeElapsed;
		if(onScreenTime > BIDI_TEST_AUTO_CLOSE_TIME)
		{
			testFinished = true;
		}
	}
    TestTemplate<BiDiTest>::Update(timeElapsed);
}

bool BiDiTest::TextFieldKeyPressed(UITextField* textField, int32 replacementLocation, int32 replacementLength, WideString const& replacementString)
{
    if (replacementLocation < 0 || replacementLength < 0)
	{
		staticText->SetText(L"");
		return true;
	}
	WideString resultString = textField->GetText();
	resultString.replace(replacementLocation, replacementLength, replacementString);
	staticText->SetText(resultString);
    return true;
}

void BiDiTest::TestFunction(PerfFuncData* data)
{
	sBiDiTestData* testData = (sBiDiTestData*)data->testData.userData;
	autoStaticText->SetText(testData->text);
	autoStaticText->SetTextAlign(testData->align);
	autoStaticText->SetTextUseRtlAlign(testData->useRtl);

	DVASSERT(autoStaticText->GetTextAlign() == testData->align);
	DVASSERT(autoStaticText->GetTextVisualAlign() == testData->visualAlign);
	DVASSERT(autoStaticText->GetTextIsRtl() == testData->isRtl);
}

void BiDiTest::ManualTestStartFunction(PerfFuncData * data)
{
	onScreenTime = 0.f;
	testFinished = false;
	manualStarted = true;
}

void BiDiTest::ManualTestProcessFunction(PerfFuncData * data)
{
	// Empty function. Waiting manual test timer.
}

bool BiDiTest::RunTest(int32 testNum)
{
	if(!manualStarted)
	{
		return TestTemplate<BiDiTest>::RunTest(testNum);
	}
	return testFinished;
}

void BiDiTest::ButtonPressed(BaseObject* obj, void* data, void* callerData)
{
    if (obj == testButton)
	{
		manualStarted = true;
		testFinished = true;
	}
	else if (obj == modeButton)
	{
		int32 align = textField->GetTextAlign();
		WideString cap = L"Align: ";
		if ((align & ALIGN_LEFT) > 0)
		{
			align = (align & ~ALIGN_LEFT) | ALIGN_HCENTER;
			cap.append(L"Center");
		}
		else if ((align & ALIGN_HCENTER) > 0)
		{
			align = (align & ~ALIGN_HCENTER) | ALIGN_RIGHT;
			cap.append(L"Right");
		}
		else if ((align & ALIGN_RIGHT) > 0)
		{
			align = (align & ~ALIGN_RIGHT) | ALIGN_LEFT;
			cap.append(L"Left");
		}
		textField->SetTextAlign(align);
		staticText->SetTextAlign(align);
		modeButton->SetStateText(0xFF, cap);
	}
	else if (obj == rtlButton)
	{
		textField->SetTextUseRtlAlign(!textField->GetTextUseRtlAlign());
		staticText->SetTextUseRtlAlign(!staticText->GetTextUseRtlAlign());
		rtlButton->SetStateText(0xFF, textField->GetTextUseRtlAlign() ? L"RTL align: Enabled" : L"RTL align: Disabled");
	}
}