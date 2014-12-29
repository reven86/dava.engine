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


#include "StaticWebViewTest.h"

static const float TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME = 30.0f;

StaticWebViewTest::StaticWebViewTest()
:	TestTemplate<StaticWebViewTest>("TransparentWebViewTest")
,	webView1(NULL)
,	webView2(NULL)
,	testFinished(false)
{
	RegisterFunction(this, &StaticWebViewTest::TestFunction, Format("TransparentWebViewTest"), NULL);
}

void StaticWebViewTest::LoadResources()
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

	//webView2 = new UIWebView(Rect(710, 5, 300, 250));
	//webView2->SetVisible(true);
	//webView2->SetBackgroundTransparency(true);
	//webView2->OpenURL(url);
	//AddControl(webView2);

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);

	finishTestButton = new UIButton(Rect(0, 510, 300, 30));
	finishTestButton->SetStateFont(0xFF, font);
	finishTestButton->SetStateText(0xFF, L"Finish Test");
	finishTestButton->SetStateFontColor(0xFF, Color::White);
	finishTestButton->SetDebugDraw(true);
	finishTestButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &StaticWebViewTest::ButtonPressed));
	AddControl(finishTestButton);

    setStaticButton = new UIButton(Rect(0, 510 + 300, 300, 30));
    setStaticButton->SetStateFont(0xFF, font);
    setStaticButton->SetStateText(0xFF, L"Render To Texture");
    setStaticButton->SetStateFontColor(0xFF, Color::White);
    setStaticButton->SetDebugDraw(true);
    setStaticButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &StaticWebViewTest::ButtonPressed));
    AddControl(setStaticButton);

    setNormalButton = new UIButton(Rect(0, 510 + 300 * 2, 300, 30));
    setNormalButton->SetStateFont(0xFF, font);
    setNormalButton->SetStateText(0xFF, L"Normal View");
    setNormalButton->SetStateFontColor(0xFF, Color::White);
    setNormalButton->SetDebugDraw(true);
    setNormalButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &StaticWebViewTest::ButtonPressed));
    AddControl(setNormalButton);

	SafeRelease(font);
}

void StaticWebViewTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(webView1);
	SafeRelease(webView2);
	SafeRelease(finishTestButton);
}

bool StaticWebViewTest::RunTest(int32 testNum)
{
	TestTemplate<StaticWebViewTest>::RunTest(testNum);
	return testFinished;
}

void StaticWebViewTest::DidAppear()
{
	onScreenTime = 0.f;
}

void StaticWebViewTest::Update(float32 timeElapsed)
{
	onScreenTime += timeElapsed;
	if(onScreenTime > TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME)
	{
		testFinished = true;
	}

	TestTemplate<StaticWebViewTest>::Update(timeElapsed);
}

void StaticWebViewTest::TestFunction(PerfFuncData * data)
{
	return;
}

void StaticWebViewTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}

void StaticWebViewTest::ButtonSetStatic(BaseObject *obj, void *data, void *callerData)
{
    // TODO
}

void StaticWebViewTest::ButtonSetNormal(BaseObject *obj, void *data, void *callerData)
{
    // TODO
}
