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

static const float TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME = 300.0f;

StaticWebViewTest::StaticWebViewTest()
:	TestTemplate<StaticWebViewTest>("StaticWebViewTest"),
    finishTestButton(nullptr),
    setStaticButton(nullptr),
    setNormalButton(nullptr),
    add10ToAlfaButton(nullptr),
    minus10FromAlfaButton(nullptr),
    checkTransparancyButton(nullptr),
    uncheckTransparancyButton(nullptr),
    overlapedImage(nullptr),
    webView1(nullptr),
    webView2(nullptr),
    testFinished(false),
    onScreenTime(0.f)
{
	RegisterFunction(this, &StaticWebViewTest::TestFunction,
                     Format("StaticWebViewTest"), NULL);
}

void StaticWebViewTest::LoadResources()
{
	webView1 = new UIWebView(Rect(5, 5, 700, 500));
	webView1->SetVisible(true);
    webView1->SetRenderToTexture(true);
    // only http://www.microsoft.com works with IE ole component nice
    webView1->OpenURL("http://www.microsoft.com");
	AddControl(webView1);

    overlapedImage = new UIControl(Rect(500, 0, 300, 300));
    FilePath imgPath("~res:/Gfx/UI/Rotation");
    overlapedImage->SetSprite(imgPath, 0);
    overlapedImage->SetDebugDraw(true);
    AddControl(overlapedImage);

	FilePath srcDir("~res:/TestData/TransparentWebViewTest/");
	FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + 
        "TransparentWebViewTest/";
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

	finishTestButton = new UIButton(Rect(0, 510, 300, 30));
	finishTestButton->SetStateFont(0xFF, font);
	finishTestButton->SetStateText(0xFF, L"Finish Test");
	finishTestButton->SetStateFontColor(0xFF, Color::White);
	finishTestButton->SetDebugDraw(true);
	finishTestButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, 
        &StaticWebViewTest::OnButtonPressed));
	AddControl(finishTestButton);

	setStaticButton = new UIButton(Rect(0 + 300, 510, 300, 30));
    setStaticButton->SetStateFont(0xFF, font);
    setStaticButton->SetStateText(0xFF, L"Render To Texture");
    setStaticButton->SetStateFontColor(0xFF, Color::White);
    setStaticButton->SetDebugDraw(true);
    setStaticButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, 
        &StaticWebViewTest::OnButtonSetStatic));
    AddControl(setStaticButton);

    setNormalButton = new UIButton(Rect(0 + 300 * 2, 510, 300, 30));
    setNormalButton->SetStateFont(0xFF, font);
    setNormalButton->SetStateText(0xFF, L"Normal View");
    setNormalButton->SetStateFontColor(0xFF, Color::White);
    setNormalButton->SetDebugDraw(true);
    setNormalButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, 
        &StaticWebViewTest::OnButtonSetNormal));
    AddControl(setNormalButton);

    

    add10ToAlfaButton = new UIButton(Rect(0 + 300 * 1, 510 + 30, 300, 30));
    add10ToAlfaButton->SetStateFont(0xFF, font);
    add10ToAlfaButton->SetStateText(0xFF, L"+10 to Alfa");
    add10ToAlfaButton->SetStateFontColor(0xFF, Color::White);
    add10ToAlfaButton->SetDebugDraw(true);
    add10ToAlfaButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, 
        &StaticWebViewTest::OnButtonAdd10ToAlfa));
    AddControl(add10ToAlfaButton);

    minus10FromAlfaButton = new UIButton(Rect(0 + 300 * 2, 510 + 30, 300, 30));
    minus10FromAlfaButton->SetStateFont(0xFF, font);
    minus10FromAlfaButton->SetStateText(0xFF, L"-10 to Alfa");
    minus10FromAlfaButton->SetStateFontColor(0xFF, Color::White);
    minus10FromAlfaButton->SetDebugDraw(true);
    minus10FromAlfaButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, 
        Message(this, &StaticWebViewTest::OnButtonMinus10FromAlfa));
    AddControl(minus10FromAlfaButton);

    checkTransparancyButton = new UIButton(
        Rect(0 + 300 * 1, 510 + 30 * 2, 300, 30));
    checkTransparancyButton->SetStateFont(0xFF, font);
    checkTransparancyButton->SetStateText(0xFF, L"set Transparent Background");
    checkTransparancyButton->SetStateFontColor(0xFF, Color::White);
    checkTransparancyButton->SetDebugDraw(true);
    checkTransparancyButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, 
        Message(this, &StaticWebViewTest::OnButtonCheckTransparancy));
    AddControl(checkTransparancyButton);

    uncheckTransparancyButton = new UIButton(
        Rect(0 + 300 * 2, 510 + 30 * 2, 300, 30));
    uncheckTransparancyButton->SetStateFont(0xFF, font);
    uncheckTransparancyButton->SetStateText(0xFF,
                                            L"unset Transparent Background");
    uncheckTransparancyButton->SetStateFontColor(0xFF, Color::White);
    uncheckTransparancyButton->SetDebugDraw(true);
    uncheckTransparancyButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE,
        Message(this, &StaticWebViewTest::OnButtonUncheckTransparancy));
    AddControl(uncheckTransparancyButton);

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

void StaticWebViewTest::TestFunction(PerfFuncData *)
{
	return;
}

void StaticWebViewTest::OnButtonPressed(BaseObject *, void *, void *)
{
	testFinished = true;
}

void StaticWebViewTest::OnButtonSetStatic(BaseObject *, void *, void *)
{
    webView1->SetRenderToTexture(true);
}

void StaticWebViewTest::OnButtonSetNormal(BaseObject *, void *, void *)
{
    webView1->SetRenderToTexture(false);
}

void StaticWebViewTest::OnButtonAdd10ToAlfa(BaseObject *obj, void *data, 
    void *callerData)
{
    Sprite* spr = webView1->GetSprite();
    UIControlBackground* back = webView1->GetBackground();
    if (spr)
    {
        Color color = back->GetColor();
        color.a += 0.1f;
        color.a = color.a <= 1.0f ? color.a : 1.0f;
        back->SetColor(color);
    }
}

void StaticWebViewTest::OnButtonMinus10FromAlfa(BaseObject *obj, void *data,
    void *callerData)
{
    Sprite* spr = webView1->GetSprite();
    UIControlBackground* back = webView1->GetBackground();
    if (spr)
    {
        Color color = back->GetColor();
        color.a -= 0.1f;
        color.a = color.a <= 1.0f ? color.a : 1.0f;
        back->SetColor(color);
    }
}

void StaticWebViewTest::OnButtonCheckTransparancy(BaseObject *obj, void *data,
                                                  void *callerData)
{
    webView1->SetBackgroundTransparency(true);
}

void StaticWebViewTest::OnButtonUncheckTransparancy(BaseObject *obj, void *data,
                                                    void *callerData)
{
    webView1->SetBackgroundTransparency(false);
}
