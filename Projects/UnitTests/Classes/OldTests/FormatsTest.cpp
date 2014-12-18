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



#include "FormatsTest.h"
#include "Render/PixelFormatDescriptor.h"

static const float TEST_TIME = 30.0f;

FormatsTest::FormatsTest()
: TestTemplate<FormatsTest>("FormatsTest")
{
    onScreenTime = 0.f;
    testFinished = false;
    
    RegisterFunction(this, &FormatsTest::TestFunction, "FormatsTest", NULL);
}

void FormatsTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));

    int32 columnCount = 6;
    int32 rowCount = (FORMAT_COUNT-1) / columnCount;
    if(0 != FORMAT_COUNT % columnCount)
    {
        ++rowCount;
    }

    float32 size = Min(GetSize().x / columnCount, GetSize().y / rowCount);
    
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);


    for(int32 i = FORMAT_RGBA8888; i < FORMAT_COUNT; ++i)
    {
		if(i == 13) continue; // FORMAT_DXT1NM was deleted

        int32 y = (i-1) / columnCount;
        int32 x = (i-1) % columnCount;
        
        String formatName = PixelFormatDescriptor::GetPixelFormatString((PixelFormat)i);
        
        UIControl *c = new UIControl(Rect(x*size, y*size, size - 2, size - 2));
        c->SetSprite(Format("~res:/TestData/FormatTest/%s/number_0", formatName.c_str()), 0);
        c->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
        
        UIStaticText *text = new UIStaticText(Rect(0, c->GetSize().y - 30, c->GetSize().x, 30));

        text->SetText(StringToWString(formatName));
        text->SetFont(font);
        text->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

        
        c->AddControl(text);
        AddControl(c);

        SafeRelease(text);
        SafeRelease(c);
    }

	finishTestBtn = new UIButton(Rect(10, 700, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
    finishTestBtn->SetStateFontColor(0xFF, Color::White);
	finishTestBtn->SetStateText(0xFF, L"Finish test");

	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &FormatsTest::ButtonPressed));
	AddControl(finishTestBtn);
    
    SafeRelease(font);
}


void FormatsTest::UnloadResources()
{
    RemoveAllControls();
	SafeRelease(finishTestBtn);
}


void FormatsTest::TestFunction(PerfFuncData * data)
{
	return;
}

void FormatsTest::DidAppear()
{
    onScreenTime = 0.f;
}

void FormatsTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > TEST_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<FormatsTest>::Update(timeElapsed);
}

bool FormatsTest::RunTest(int32 testNum)
{
	TestTemplate<FormatsTest>::RunTest(testNum);
	return testFinished;
}

void FormatsTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	if (obj == finishTestBtn)
	{
		testFinished = true;
	}
}
