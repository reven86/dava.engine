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


#include "Tests/StaticWebViewTest.h"

#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Utils/Utils.h"

namespace
{

const String htmlCuteCats =
    "<html>"
    "  <head>"
    "    <link rel='stylesheet' type='text/css' href='test.css'>"
    "    <script type = 'text/javascript'>"
    "      function doSomething() {"
    "          var x = document.getElementById('mydiv');"
    "          x.innerHTML += '<h3>Hello from JS</h3>';"
    "      }"
    "    </script>"
    "  </head>"
    "  <body>"
    "      <h1>Cute cats picture</h1><br/>"
    "      <br/>"
    "      <img src='cute-cat-picture.jpg'/>"
    "      <div id='mydiv'>"
    "      </div>"
    "  </body>"
    "</html>";

const WideString htmlString = 
    L"<html>"
    L"  <head>"
    L"  </head>"
    L"  <body style='color: #d0e4fe'>"
    L"      <h1>This is a WebView</h1>"
    L"      <a href='http://www.turion.by'>click me</a><br/>"
    L"      <a href='https://wargaming.net'>click me</a><br/>"
    L"  </body>"
    L"</html>";

}   // unnamed namespace

class MyWebViewDelegate : public IUIWebViewDelegate
{
public:
    eAction URLChanged(UIWebView* webview, const String& newURL, bool isRedirectedByMouseClick) override
    {
        Logger::Debug("MyWebViewDelegate::URLChanged: %s", newURL.c_str());
        return eAction::PROCESS_IN_WEBVIEW;
    }

    void OnExecuteJScript(UIWebView* webview, const String& result) override
    {
        Logger::Debug("MyWebViewDelegate::OnExecuteJScript: %s", result.c_str());
    }

    void PageLoaded(UIWebView* webview) override
    {
        Logger::Debug("MyWebViewDelegate::PageLoaded");
    }

    void SwipeGesture(bool left) override {}
};

StaticWebViewTest::StaticWebViewTest()
    : BaseScreen("StaticWebViewTest")
{}

void StaticWebViewTest::LoadResources()
{
    webviewDelegate = new MyWebViewDelegate;

    webView1 = new UIWebView(Rect(5, 5, 400, 300));
    webView1->SetVisible(true);
    webView1->SetRenderToTexture(true);
    webView1->SetDebugDraw(true);
    webView1->SetDelegate(webviewDelegate);
    webView1->OpenURL("http://en.cppreference.com/");
    AddControl(webView1);

    webView2 = new UIWebView(Rect(410, 50, 400, 300));
    webView2->SetVisible(true);
    webView2->SetDebugDraw(true);
    webView2->OpenFromBuffer(htmlCuteCats, FileSystem::Instance()->GetCurrentExecutableDirectory() + "Data/TestData/TransparentWebViewTest/");
    AddControl(webView2);

    webView3 = new UIWebView(Rect(820, 70, 400, 300));
    webView3->SetVisible(true);
    webView3->SetRenderToTexture(true);
    webView3->SetDebugDraw(true);
    webView3->LoadHtmlString(htmlString);
    AddControl(webView3);

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(20);

    const float32 w = 40;

    overlapedImage = new UIControl(Rect(500, 0, 300, 300));
    FilePath imgPath("~res:/Gfx/UI/Rotation");
    overlapedImage->SetSprite(imgPath, 0);
    overlapedImage->SetDebugDraw(true);
    AddControl(overlapedImage);

    FilePath srcDir("~res:/TestData/TransparentWebViewTest/");
    FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "TransparentWebViewTest/";
    FileSystem::Instance()->DeleteDirectory(cpyDir);
    FileSystem::Instance()->CreateDirectory(cpyDir);
    FilePath srcFile = srcDir + "test.html";
    FilePath cpyFile = cpyDir + "test.html";
    FileSystem::Instance()->CopyFile(srcFile, cpyFile);

    setStaticButton           = CreateUIButton(font, Rect(0 + 300, 510, 300, w), L"Render To Texture", &StaticWebViewTest::OnButtonSetStatic);
    setNormalButton           = CreateUIButton(font, Rect(0 + 300 * 2, 510, 300, w), L"Normal View", &StaticWebViewTest::OnButtonSetNormal);
    add10ToAlfaButton         = CreateUIButton(font, Rect(0 + 300 * 1, 510 + w, 300, w), L"+10 to Alfa", &StaticWebViewTest::OnButtonAdd10ToAlfa);
    minus10FromAlfaButton     = CreateUIButton(font, Rect(0 + 300 * 2, 510 + w, 300, w), L"-10 to Alfa", &StaticWebViewTest::OnButtonMinus10FromAlfa);
    checkTransparancyButton   = CreateUIButton(font, Rect(0 + 300 * 1, 510 + w * 2, 300, w), L"set Transparent Background", &StaticWebViewTest::OnButtonCheckTransparancy);
    uncheckTransparancyButton = CreateUIButton(font, Rect(0 + 300 * 2, 510 + w * 2, 300, w), L"unset Transparent Background", &StaticWebViewTest::OnButtonUncheckTransparancy);
    executeJSButton           = CreateUIButton(font, Rect(0 + 300 * 1, 510 + w * 3, 300, w), L"exec JS", &StaticWebViewTest::OnButtonExecJS);
    loadHTMLString            = CreateUIButton(font, Rect(0 + 300 * 2, 510 + w * 3, 300, w), L"load HTML String", &StaticWebViewTest::OnLoadHTMLString);
    setVisibleButton          = CreateUIButton(font, Rect(0 + 300 * 1, 510 + w * 4, 300, w), L"Show", &StaticWebViewTest::OnButtonVisible);
    setHideButton             = CreateUIButton(font, Rect(0 + 300 * 2, 510 + w * 4, 300, w), L"Hide", &StaticWebViewTest::OnButtonHide);

    SafeRelease(font);

    BaseScreen::LoadResources();
}

void StaticWebViewTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(webView1);
    SafeRelease(webView2);
    SafeRelease(webView3);

    SafeDelete(webviewDelegate);

    SafeRelease(setStaticButton);
    SafeRelease(setNormalButton);
    SafeRelease(add10ToAlfaButton);
    SafeRelease(minus10FromAlfaButton);
    SafeRelease(checkTransparancyButton);
    SafeRelease(uncheckTransparancyButton);
    SafeRelease(executeJSButton);
    SafeRelease(loadHTMLString);
    SafeRelease(setVisibleButton);
    SafeRelease(setHideButton);
}

void StaticWebViewTest::OnButtonSetStatic(BaseObject *, void *, void *)
{
    webView1->SetRenderToTexture(true);
    webView2->SetRenderToTexture(true);
    webView3->SetRenderToTexture(true);
}

void StaticWebViewTest::OnButtonSetNormal(BaseObject *, void *, void *)
{
    webView1->SetRenderToTexture(false);
    webView2->SetRenderToTexture(false);
    webView3->SetRenderToTexture(false);
}

void StaticWebViewTest::OnButtonAdd10ToAlfa(BaseObject *obj, void *data, void *callerData)
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

void StaticWebViewTest::OnButtonMinus10FromAlfa(BaseObject *obj, void *data, void *callerData)
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

void StaticWebViewTest::OnButtonCheckTransparancy(BaseObject *obj, void *data, void *callerData)
{
    webView1->SetBackgroundTransparency(true);
    webView2->SetBackgroundTransparency(true);
    webView3->SetBackgroundTransparency(true);
}

void StaticWebViewTest::OnButtonUncheckTransparancy(BaseObject *obj, void *data, void *callerData)
{
    webView1->SetBackgroundTransparency(false);
    webView2->SetBackgroundTransparency(false);
    webView3->SetBackgroundTransparency(false);
}

UIButton* StaticWebViewTest::CreateUIButton(Font* font, const Rect& rect, const WideString& str,
                                            void (StaticWebViewTest::*targetFunction)(BaseObject*, void*, void*))
{
    UIButton* button = new UIButton(rect);
    button->SetStateFont(0xFF, font);
    button->SetStateText(0xFF, str);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetDebugDraw(true);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, targetFunction));
    AddControl(button);
    return button;
}

void StaticWebViewTest::OnButtonExecJS(BaseObject* obj, void*, void*)
{
    webView1->ExecuteJScript(
            "document.body.innerHTML = \"<H1>Hi from JS!</H1>"
            "<P>Test only test</P>\"");
    webView2->ExecuteJScript("doSomething()");
}

void StaticWebViewTest::OnLoadHTMLString(BaseObject* obj, void*, void*)
{
    static bool switchHtml = false;
    switchHtml = !switchHtml;

    if (switchHtml)
    {
        webView1->LoadHtmlString(
            L"<HTML>"
            L"   <HEAD></HEAD>"
            L"<BODY bgcolor='#E6E6FA'>"
            L"   <H1>Hi</H1>"
            L"   <P>This is HTML document with explicitly set background color</P>"
            L"</BODY>"
            L"</HTML>");
    }
    else
    {
        webView1->LoadHtmlString(
            L"<HTML>"
            L"   <HEAD></HEAD>"
            L"<BODY text='blue'>"
            L"   <H1>Hi</H1>"
            L"   <P>This is HTML document with background color not set</P>"
            L"</BODY>"
            L"</HTML>");
    }
    webView3->LoadHtmlString(htmlString);
}

void StaticWebViewTest::OnButtonVisible(BaseObject*, void*, void*)
{
    webView1->SetVisible(true);
    webView2->SetVisible(true);
    webView3->SetVisible(true);
}

void StaticWebViewTest::OnButtonHide(BaseObject*, void*, void*)
{
    webView1->SetVisible(false);
    webView2->SetVisible(false);
    webView3->SetVisible(false);
}
