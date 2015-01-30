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


//
//  DFFontTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "DFFontTest.h"

using namespace DAVA;

static const float INPUT_TEST_AUTO_CLOSE_TIME = 30.0f;

DFFontTest::DFFontTest() :
 TestTemplate<DFFontTest>("DFFontTest")
{
	testButton = NULL;
	
	onScreenTime = 0.0f;
	testFinished = false;
	
	RegisterFunction(this, &DFFontTest::TestFunction, Format("DFFontTest"), NULL);
}

void DFFontTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
	
	WideString testText = L"Distance font test";
	
	{
		DFFont* dfFont = DFFont::Create("~res:/DFFont/testFont.df");
		dfFont->SetSize(20);
		UIStaticText* staticText = new UIStaticText();
		staticText->SetFont(dfFont);
		staticText->SetRect(Rect(100, 15, 1000, 100));
		staticText->SetText(testText);
		AddControl(staticText);

        SafeRelease(dfFont);
        SafeRelease(staticText);
	}
	
	{
		DFFont* dfFont = DFFont::Create("~res:/DFFont/testFont.df");
		dfFont->SetSize(48);
		UIStaticText* staticText = new UIStaticText();
		staticText->SetFont(dfFont);
		staticText->SetRect(Rect(100, 250, 1000, 100));
		staticText->SetText(testText);
		AddControl(staticText);

        SafeRelease(dfFont);
        SafeRelease(staticText);
	}
	
	{
		DFFont* dfFont = DFFont::Create("~res:/DFFont/testFont.df");
		dfFont->SetSize(65);
		UIStaticText* staticText = new UIStaticText();
		staticText->SetFont(dfFont);
		staticText->SetRect(Rect(100, 450, 1000, 100));
		staticText->SetText(testText);
		AddControl(staticText);

        SafeRelease(dfFont);
        SafeRelease(staticText);
	}
	
	{
		UIStaticText* staticText1 = new UIStaticText();
		staticText1->SetFont(font);
		staticText1->SetRect(Rect(10, 15, 300, 30));
		staticText1->SetText(L"FTFont test");
		AddControl(staticText1);

        SafeRelease(staticText1);
	}

	testButton = new UIButton(Rect(0, 600, 300, 30));
	testButton->SetStateFont(0xFF, font);
	//testButton->SetStateFontColor(0xFF, Color::White());
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DFFontTest::ButtonPressed));
	AddControl(testButton);

	SafeRelease(font);
}

void DFFontTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(testButton);
}

void DFFontTest::TestFunction(PerfFuncData * data)
{
	return;
}

void DFFontTest::DidAppear()
{
    onScreenTime = 0.f;
}

void DFFontTest::Update(float32 timeElapsed)
{
    //onScreenTime += timeElapsed;
    if(onScreenTime > INPUT_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }

    TestTemplate<DFFontTest>::Update(timeElapsed);
}

bool DFFontTest::RunTest(int32 testNum)
{
	TestTemplate<DFFontTest>::RunTest(testNum);
	return testFinished;
}

void DFFontTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	if (obj == testButton)
	{
		testFinished = true;
	}
}