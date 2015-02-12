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


#include "TransparentWebViewTest.h"

static const float TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME = 30.0f;

TransparentWebViewTest::TransparentWebViewTest()
:	TestTemplate<TransparentWebViewTest>("TransparentWebViewTest")
,	webView1(NULL)
,	webView2(NULL)
,	testFinished(false)
{
	RegisterFunction(this, &TransparentWebViewTest::TestFunction, Format("TransparentWebViewTest"), NULL);
}

void TransparentWebViewTest::LoadResources()
{
	webView1 = new UIWebView(Rect(5, 5, 700, 500));
	webView1->SetVisible(true);
    webView1->SetRenderToTexture(true);
	webView1->OpenURL("https://ru.wikipedia.org/");
	AddControl(webView1);

	FilePath srcDir("~res:/TestData/TransparentWebViewTest/");
	FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "TransparentWebViewTest/";
	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);
	FilePath srcFile = srcDir + "test.html";
	FilePath cpyFile = cpyDir + "test.html";
	FileSystem::Instance()->CopyFile(srcFile, cpyFile);
	String url = "file:///" + cpyFile.GetAbsolutePathname();

	webView2 = new UIWebView(Rect(710, 5, 300, 250));
	webView2->SetVisible(true);
	webView2->SetBackgroundTransparency(true);
	webView2->OpenURL(url);
	AddControl(webView2);

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);

	testButton = new UIButton(Rect(0, 510, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetStateFontColor(0xFF, Color::White);
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TransparentWebViewTest::ButtonPressed));
	AddControl(testButton);

	SafeRelease(font);
}

void TransparentWebViewTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(webView1);
	SafeRelease(webView2);
	SafeRelease(testButton);
}

bool TransparentWebViewTest::RunTest(int32 testNum)
{
	TestTemplate<TransparentWebViewTest>::RunTest(testNum);
	return testFinished;
}

void TransparentWebViewTest::DidAppear()
{
	onScreenTime = 0.f;
}

void TransparentWebViewTest::Update(float32 timeElapsed)
{
	onScreenTime += timeElapsed;
	if(onScreenTime > TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME)
	{
		testFinished = true;
	}

	TestTemplate<TransparentWebViewTest>::Update(timeElapsed);
}

void TransparentWebViewTest::TestFunction(PerfFuncData * data)
{
	return;
}

void TransparentWebViewTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}
