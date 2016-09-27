#include "Tests/WebViewTest.h"

using namespace DAVA;

const WideString htmlContent =
L"<html>"
L"<head></head>"
L"<body text='white'>"
L"Some test text"
L"</body>"
L"</html>";

WebViewTest::WebViewTest(GameCore& gameCore)
    : BaseScreen(gameCore, "WebViewTest")
    , webView(nullptr)
    , bgStubPanel(nullptr)
    , updateWait(false)
{
}

void WebViewTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    webView = new UIWebView(Rect(10, 10, 400, 200));
    webView->LoadHtmlString(htmlContent);
    webView->SetBackgroundTransparency(true);
    AddControl(webView);

    bgStubPanel = new UIControl(Rect(10, 10, 400, 200));
    bgStubPanel->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    bgStubPanel->GetBackground()->SetColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    bgStubPanel->GetBackground()->SetColor(Color(1, 0, 0, 1));
    bgStubPanel->SetVisibilityFlag(false);
    AddControl(bgStubPanel);

    ScopedPtr<UIButton> visibleBtn(new UIButton(Rect(440, 10, 200, 50)));
    visibleBtn->SetStateFont(0xFF, font);
    visibleBtn->SetStateText(0xFF, L"Show/hide with sleep");
    visibleBtn->SetDebugDraw(true);
    visibleBtn->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &WebViewTest::OnVisibleClick));
    AddControl(visibleBtn);
}

void WebViewTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    SafeRelease(webView);
    SafeRelease(bgStubPanel);
}

void WebViewTest::Update(float32 delta)
{
    // Simulate time lag between screens or tabs with loading resources
    if (updateWait)
    {
        Thread::Sleep(500);
        updateWait = false;
    }

    BaseScreen::Update(delta);
}

void WebViewTest::OnVisibleClick(BaseObject* sender, void* data, void* callerData)
{
    webView->SetVisibilityFlag(!webView->GetVisibilityFlag());
    bgStubPanel->SetVisibilityFlag(!webView->GetVisibilityFlag());

    updateWait = true;
}
