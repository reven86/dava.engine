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
    executeJSButton(nullptr),
    loadHTMLString(nullptr),
    overlapedImage(nullptr),
    setVisibleButton(nullptr),
    setHideButton(nullptr),
    webView1(nullptr),
    webView2(nullptr),
    testFinished(false),
    onScreenTime(0.f)
{
	RegisterFunction(this, &StaticWebViewTest::TestFunction,
                     Format("StaticWebViewTest"), nullptr);
}

void StaticWebViewTest::LoadResources()
{
	webView1 = new UIWebView(Rect(5, 5, 700, 500));
	webView1->SetVisible(true);
    webView1->SetRenderToTexture(true);
    webView1->SetDebugDraw(true);
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
    webView2->SetDebugDraw(true);
	webView2->SetBackgroundTransparency(true);
    webView2->OpenURL(url);
	AddControl(webView2);

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);

	const float32 w = 40;

    CreateUIButton(finishTestButton, font, Rect(0, 510, 300, w),
        L"Finish Test", &StaticWebViewTest::OnButtonPressed);

    CreateUIButton(setStaticButton, font, Rect(0 + 300, 510, 300, w),
        L"Render To Texture", &StaticWebViewTest::OnButtonSetStatic);

    CreateUIButton(setNormalButton, font, Rect(0 + 300 * 2, 510, 300, w),
        L"Normal View", &StaticWebViewTest::OnButtonSetNormal);
    
    CreateUIButton(add10ToAlfaButton, font, Rect(0 + 300 * 1, 510 + w, 300, w),
        L"+10 to Alfa", &StaticWebViewTest::OnButtonAdd10ToAlfa);

    CreateUIButton(minus10FromAlfaButton, font, 
        Rect(0 + 300 * 2, 510 + w, 300, w),
        L"-10 to Alfa", &StaticWebViewTest::OnButtonMinus10FromAlfa);

    CreateUIButton(checkTransparancyButton, font,
        Rect(0 + 300 * 1, 510 + w * 2, 300, w),
        L"set Transparent Background", 
        &StaticWebViewTest::OnButtonCheckTransparancy);

    CreateUIButton(uncheckTransparancyButton, font,
        Rect(0 + 300 * 2, 510 + w * 2, 300, w),
        L"unset Transparent Background",
        &StaticWebViewTest::OnButtonUncheckTransparancy);

    CreateUIButton(executeJSButton, font,
            Rect(0 + 300 * 1, 510 + w * 3, 300, w), L"exec JS",
            &StaticWebViewTest::OnButtonExecJS);

    CreateUIButton(loadHTMLString, font,
            Rect(0 + 300 * 2, 510 + w * 3, 300, w), L"load HTML String",
            &StaticWebViewTest::OnLoadHTMLString);

    CreateUIButton(setVisibleButton, font,
        Rect(0 + 300 * 1, 510 + w * 4, 300, w), L"Show",
        &StaticWebViewTest::OnButtonVisible);

    CreateUIButton(setHideButton, font,
        Rect(0 + 300 * 2, 510 + w * 4, 300, w), L"Hide",
        &StaticWebViewTest::OnButtonHide);

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
        color.a = Min(1.0f, color.a);
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
        color.a = Max(0.f, color.a);
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

void StaticWebViewTest::CreateUIButton(UIButton*& button, Font * font, 
    const Rect& rect, const WideString& str, 
    void (StaticWebViewTest::*targetFunction)(BaseObject*, void*, void*))
{
    button = new UIButton(rect);
    button->SetStateFont(0xFF, font);
    button->SetStateText(0xFF, str);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetDebugDraw(true);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, 
        Message(this, targetFunction));
    AddControl(button);
}

void StaticWebViewTest::OnButtonExecJS(BaseObject* obj, void*, void*)
{
    webView1->ExecuteJScript(
            "document.body.innerHTML = \"<H1>Hi from JS!</H1>"
            "<P>Test only test</P>\"");
}

void StaticWebViewTest::OnLoadHTMLString(BaseObject* obj, void*, void*)
{
    static bool switchHtml = false;
    switchHtml = !switchHtml;

    if (switchHtml)
    {
        webView1->LoadHtmlString(
            L"<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
            L"<HTML>\n"
            L"   <HEAD>\n"
            L"      <TITLE>\n"
            L"         A Small Hello \n"
            L"      </TITLE>\n"
            L"   </HEAD>\n"
            L"<BODY>\n"
            L"   <H1>Hi</H1>\n"
            L"   <P>This is very minimal \"hello world\" HTML document.</P> \n"
            L"</BODY>\n"
            L"</HTML>\n");
    }
    else
    {
        webView1->LoadHtmlString(
            L"<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
            L"<HTML>\n"
            L"   <HEAD>\n"
            L"      <TITLE>\n"
            L"         Second A Small Hello \n"
            L"      </TITLE>\n"
            L"   </HEAD>\n"
            L"<BODY bgcolor=\"#E6E6FA\">\n"
            L"   <H1>Hi</H1>\n"
            L"   <P>This is very minimal \"hello World Of Blitz\" HTML document.</P> \n"
            L"</BODY>\n"
            L"</HTML>\n");
    }
}

void StaticWebViewTest::OnButtonVisible(BaseObject*, void*, void*)
{
    webView1->SetVisible(true);
}

void StaticWebViewTest::OnButtonHide(BaseObject*, void*, void*)
{
    webView1->SetVisible(false);
}
