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
//  InputTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "DPITest.h"

using namespace DAVA;

float32 DPITest::AUTO_CLOSE_TIME = 30.f;

DPITest::DPITest() :
 TestTemplate<DPITest>("DPITest")
{
	staticText = NULL;
	testButton = NULL;
	
	testFinished = false;
	
	RegisterFunction(this, &DPITest::TestFunction, Format("DPITest"), NULL);
    
    onScreenTime = 0.f;
}

void DPITest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
	
	staticText = new UIStaticText(Rect(0, 0, 512, 100));
	staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
	staticText->SetText(L"textField");
	staticText->SetDebugDraw(true);
	//staticText->SetDelegate(new UITextFieldDelegate());
    String str(Format("Detected DPI: %d", DAVA::Core::Instance()->GetScreenDPI()));
    staticText->SetText(StringToWString(str));
	AddControl(staticText);
	

	testButton = new UIButton(Rect(0, 300, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateFontColor(0xFF, Color::White);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DPITest::ButtonPressed));

	AddControl(testButton);
	SafeRelease(font);
}

void DPITest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(testButton);
	SafeRelease(staticText);
}


void DPITest::DidAppear()
{
    onScreenTime = 0.f;
}

void DPITest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<DPITest>::Update(timeElapsed);
}

void DPITest::TestFunction(PerfFuncData * data)
{
	return;
}

bool DPITest::RunTest(int32 testNum)
{
	TestTemplate<DPITest>::RunTest(testNum);
	return testFinished;
}


void DPITest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}

