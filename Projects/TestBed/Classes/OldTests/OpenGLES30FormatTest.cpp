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


#include "OpenGLES30FormatTest.h"
#include "FileSystem/File.h"


static const String FORMAT_NAMES[] =
{
    "RG11",
    "R11",
    "ETC2_RGB",
    "ETC2_RGBA",
    "ETC2_RGB_A1",
    "PVRTC2_2",
    "PVRTC2_4",
};

static const uint32 FORMATS_COUNT = 7;

static const float32 AUTO_CLOSE_TIME = 30.0f;
static const float32 TEST_TIME = AUTO_CLOSE_TIME/FORMATS_COUNT;

OpenGLES30FormatTest::OpenGLES30FormatTest()
    :	TestTemplate<OpenGLES30FormatTest>("OpenGLES30FormatTest")
    ,   testFinished(false)
    ,   source(NULL)
    ,   decodedSource(NULL)
    ,   encodedSource(NULL)
    ,   formatName(NULL)
    ,   currentFormatIndex(0)
{
	RegisterFunction(this, &OpenGLES30FormatTest::TestFunction, Format("OpenGLES30FormatTest test"), NULL);
}

void OpenGLES30FormatTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);

	UIButton *testButton = new UIButton(Rect(0, 0, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
 	testButton->SetStateFontColor(0xFF, Color::White);
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &OpenGLES30FormatTest::ButtonPressed));
	AddControl(testButton);
    testButton->Release();

    formatName = new UIStaticText(Rect(0, 30, 300, 30));
    formatName->SetFont(font);
    formatName->SetTextColor(Color::White);
    AddControl(formatName);
    
    source = new UIControl(Rect(0, 60, 256, 256));
    AddControl(source);

    decodedSource = new UIControl(Rect(0, 330, 256, 256));
    AddControl(decodedSource);
    
    encodedSource = new UIControl(Rect(300, 330, 256, 256));
    AddControl(encodedSource);
    
    LoadFormat("RG11");
}

void OpenGLES30FormatTest::LoadFormat(const String & format)
{
    if(currentFormatName == format) return;
    
    currentFormatName = format;
    
    formatName->SetText(StringToWString(format));
    
    Sprite *sprSource = Sprite::CreateFromSourceFile(Format("~res:/TestData/OpenGLES30FormatTest/%s/source.png", format.c_str()));
    Sprite *sprEncoded = Sprite::CreateFromSourceFile(Format("~res:/TestData/OpenGLES30FormatTest/%s/source_encoded.pvr", format.c_str()));
    Sprite *sprDecoded = Sprite::CreateFromSourceFile(Format("~res:/TestData/OpenGLES30FormatTest/%s/source_decoded.png", format.c_str()));
    
    source->SetSprite(sprSource, 0);
    encodedSource->SetSprite(sprEncoded, 0);
    decodedSource->SetSprite(sprDecoded, 0);
    
    SafeRelease(sprSource);
    SafeRelease(sprEncoded);
    SafeRelease(sprDecoded);
}

void OpenGLES30FormatTest::UnloadResources()
{
    RemoveAllControls();
    
    SafeRelease(formatName);
    SafeRelease(source);
    SafeRelease(encodedSource);
    SafeRelease(decodedSource);
}

bool OpenGLES30FormatTest::RunTest(int32 testNum)
{
	TestTemplate<OpenGLES30FormatTest>::RunTest(testNum);
	return testFinished;
}

void OpenGLES30FormatTest::TestFunction(TestTemplate<OpenGLES30FormatTest>::PerfFuncData *data)
{
    LoadFormat(FORMAT_NAMES[currentFormatIndex]);
    
    currentFormatIndex = Clamp((uint32)(onScreenTime / TEST_TIME), 0u, FORMATS_COUNT - 1);
}

void OpenGLES30FormatTest::DidAppear()
{
	onScreenTime = 0.f;
}

void OpenGLES30FormatTest::Update(float32 timeElapsed)
{
	onScreenTime += timeElapsed;
	if(onScreenTime > AUTO_CLOSE_TIME)
	{
		testFinished = true;
	}
    
	TestTemplate<OpenGLES30FormatTest>::Update(timeElapsed);
}

void OpenGLES30FormatTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}
