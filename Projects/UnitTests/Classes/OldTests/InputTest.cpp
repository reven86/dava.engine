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

#include "InputTest.h"
#include "UI/UIMovieView.h"

using namespace DAVA;

static const float INPUT_TEST_AUTO_CLOSE_TIME = 30.0f;


class UIWebViewDelegate: public IUIWebViewDelegate
{
	eAction URLChanged(UIWebView* webview, const String& newURL, bool isRedirectedByMouseClick) override;
    void OnExecuteJScript(UIWebView* webview, const String& result) override;
    void PageLoaded(UIWebView* webview) override;
};

class UIMoveableTextFieldDelegate : public UITextFieldDelegate
{
public:
	UIMoveableTextFieldDelegate(UITextField* textField)
	{
		this->moveableTextField = textField;
	}
	
	virtual void OnKeyboardShown(const Rect& keyboardRect)
	{
		this->initialTextFieldRect = this->moveableTextField->GetRect();
		Rect newRect = this->initialTextFieldRect;
		
		newRect.y = keyboardRect.y - newRect.dy;

		this->moveableTextField->SetRect(newRect);
	}

	virtual void OnKeyboardHidden()
	{
		this->moveableTextField->SetRect(initialTextFieldRect);
	}

protected:
	UITextField* moveableTextField;
	Rect initialTextFieldRect;
};

IUIWebViewDelegate::eAction UIWebViewDelegate::URLChanged(UIWebView* webview, const String& newURL, bool isInitiatedByUser)
{
	if(isInitiatedByUser)
	{
		DAVA::Logger::Debug("Link %s from browser", newURL.c_str());
	}
	else
	{
		DAVA::Logger::Debug("Link %s from source code", newURL.c_str());
	}

	if (newURL.find("google.com.ua") != String::npos)
	{
		return IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER;
	}
	else if (newURL.find("bash.im") != String::npos)
	{
		return IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER;
	}
	else if (newURL.find("microsoft.com") != String::npos)
	{
		return IUIWebViewDelegate::NO_PROCESS;
	}

	return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
}

void UIWebViewDelegate::OnExecuteJScript(DAVA::UIWebView* webview, const String& result)
{
    Logger::Debug("OnExecuteJScript result:%s", result.c_str());
}

void UIWebViewDelegate::PageLoaded(UIWebView* webview)
{
	webview->SetVisible(true);
}

InputTest::InputTest() :
 TestTemplate<InputTest>("InputTest")
{
	textField = NULL;
	passwordTextField = NULL;
	staticText = NULL;
    
	testButton = NULL;
	removeFromParentButton = NULL;
    disableInEventButton = NULL;
	
	onScreenTime = 0.0f;
	testFinished = false;

    cursorUpdateTime = 0.0f;
    cursorMoveForward = false;

	RegisterFunction(this, &InputTest::TestFunction, Format("InputTest"), NULL);
}

void InputTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Texture* texture = Texture::CreateFromFile("~res:/TestData/InputTest/rect2.png");
	Sprite* spr = Sprite::CreateFromTexture(texture,0,0,static_cast<float32>(texture->width),
		static_cast<float32>(texture->height));

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
	
	passwordTextField = new UITextField(Rect(0, 30, 512, 50));
#ifdef __DAVAENGINE_IPHONE__
#else
	passwordTextField->SetFont(font);
#endif
    passwordTextField->SetTextColor(Color::White);

	passwordTextField->SetSprite(spr,0);
    passwordTextField->SetSpriteAlign(ALIGN_RIGHT);
	passwordTextField->SetTextAlign(ALIGN_LEFT | ALIGN_BOTTOM);
	passwordTextField->SetText(L"");
	passwordTextField->SetDebugDraw(true);
	passwordTextField->SetDelegate(new UITextFieldDelegate());
	passwordTextField->SetIsPassword(true);
	passwordTextField->SetDelegate(this);
//	passwordTextField->SetInputEnabled(false, false);
	AddControl(passwordTextField);
	
	textField = new UITextField(Rect(0, 600, 950, 40));
#ifdef __DAVAENGINE_IPHONE__
#else
	textField->SetFont(font);
#endif
    textField->SetTextColor(Color::White);

	textField->SetText(L"This field will auto-move over the keyboard.");
	textField->SetDebugDraw(true);
	textField->SetDelegate(new UIMoveableTextFieldDelegate(textField));

	textField->SetAutoCapitalizationType(DAVA::UITextField::AUTO_CAPITALIZATION_TYPE_NONE);
	textField->SetAutoCorrectionType(DAVA::UITextField::AUTO_CORRECTION_TYPE_NO);
	textField->SetKeyboardAppearanceType(DAVA::UITextField::KEYBOARD_APPEARANCE_ALERT);
	textField->SetReturnKeyType(DAVA::UITextField::RETURN_KEY_JOIN);
	textField->SetKeyboardType(DAVA::UITextField::KEYBOARD_TYPE_TWITTER);
	textField->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);

	AddControl(textField);

	textField = new UITextField(Rect(750, 10, 100, 500));
#ifdef __DAVAENGINE_IPHONE__
#else
	textField->SetFont(font);
#endif
    
    textField->SetTextColor(Color::White);

	textField->SetText(L"Length lim 12");
	textField->SetDebugDraw(true);
	textField->SetDelegate(new UITextFieldDelegate());
    textField->SetMaxLength(12);
	AddControl(textField);

	removeFromParentButton = new UIButton(Rect(320, 300, 300, 30));
	removeFromParentButton->SetStateFont(0xFF, font);
	removeFromParentButton->SetStateFontColor(0xFF, Color::White);
	removeFromParentButton->SetStateText(0xFF, L"Remove From Parent Test");
	removeFromParentButton->SetDebugDraw(true);
	removeFromParentButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::ButtonPressed));

	testButton = new UIButton(Rect(0, 300, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateFontColor(0xFF, Color::White);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::ButtonPressed));

    disableInEventButton = new UIButton(Rect(0, 340, 300, 30));
	disableInEventButton->SetStateFont(0xFF, font);
	disableInEventButton->SetStateFontColor(0xFF, Color::White);
	disableInEventButton->SetStateText(0xFF, L"Disable and check input");
	disableInEventButton->SetDebugDraw(true);
	disableInEventButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::ButtonPressed));

	staticText = new UIStaticText(Rect(0, 0, 512, 20));
	font->SetSize(10);
	staticText->SetFont(font);
    staticText->SetTextColor(Color::White);
	staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);// 12 - Rtop
	staticText->SetText(L"Type password in the field below");
	staticText->SetDebugDraw(true);
	AddControl(staticText);

    cursorPositionStaticText = new UIStaticText(Rect(0, 82, 100, 20));
	font->SetSize(10);
	cursorPositionStaticText->SetFont(font);
    cursorPositionStaticText->SetTextColor(Color::White);
	cursorPositionStaticText->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
	cursorPositionStaticText->SetDebugDraw(true);
	AddControl(cursorPositionStaticText);

	webView1 = new UIWebView(Rect(5, 105, 500, 190));
	webView1->SetVisible(false);
	delegate = new UIWebViewDelegate();
	webView1->SetDelegate((UIWebViewDelegate*)delegate);
	webView1->OpenURL("http://www.google.com");
	AddControl(webView1);

	webView2 = new UIWebView(Rect(305, 300, 440, 190));
    webView2->SetVisible(false);
    webView2->SetDelegate((UIWebViewDelegate*)delegate);
	webView2->LoadHtmlString(L"<html><head><title>Test JavaScript Title</title></head><body>LINK TO DAVA - <a id='testLink' href='http://www.davaconsulting.com/'>HELLO DAVA</a></body></html>");
	webView2->SetBounces(true);
	AddControl(webView2);

	FilePath srcDir("~res:/TestData/InputTest/");
	FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "InputTest/";
	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);
	FilePath srcFile = srcDir + "test.html";
	FilePath cpyFile = cpyDir + "test.html";
	FileSystem::Instance()->CopyFile(srcFile, cpyFile);
	String url = "file:///" + cpyFile.GetAbsolutePathname();

	//delegate = new UIWebViewDelegate();
	webView3 = new UIWebView(Rect(520, 130, 215, 135));
	webView3->SetDelegate((UIWebViewDelegate*)delegate);
	webView3->OpenURL(url);

	AddControl(webView3);

	AddControl(testButton);
	AddControl(removeFromParentButton);
    AddControl(disableInEventButton);

    SafeRelease(spr);
    SafeRelease(texture);
	SafeRelease(font);

	/*
	staticText->SetShadowColor(DAVA::Color(0xFF/255.f, 0xC4/255.f, 0xC3/255.f, 1.f));
	staticText->SetShadowOffset(DAVA::Vector2(4.0f, 4.0f));
	Color faded = staticText->GetBackground()->color;
	faded.a = 0.1f;
	staticText->ColorAnimation(faded, 2.0f, Interpolation::LINEAR);
	staticText->ShadowColorAnimation(faded, 2.0f, Interpolation::LINEAR);
	 */

	DisplayUIControlsSize();
}

void InputTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(testButton);
	SafeRelease(removeFromParentButton);
	SafeRelease(disableInEventButton);
    
	SafeRelease(textField);
	SafeRelease(passwordTextField);
	SafeRelease(staticText);

    SafeRelease(cursorPositionStaticText);

	SafeRelease(webView1);
	SafeRelease(webView2);
	SafeRelease(webView3);
	
	UIWebViewDelegate* d = (UIWebViewDelegate*)delegate;
	delete d;
}

void InputTest::TestFunction(PerfFuncData * data)
{
	return;
}

void InputTest::DidAppear()
{
    onScreenTime = 0.f;
}

void InputTest::Update(float32 timeElapsed)
{
    std::wstringstream cursorPosStream;
    cursorPosStream << "Cursor Position: " << passwordTextField->GetCursorPos();
    cursorPositionStaticText->SetText(cursorPosStream.str());

    cursorUpdateTime += timeElapsed;
    if (cursorUpdateTime > 0.5f)
    {
        uint32 cursorPos = passwordTextField->GetCursorPos();
        if (cursorMoveForward)
        {
            if (cursorPos < passwordTextField->GetText().length())
            {
                cursorPos ++;
            }
            else
            {
                cursorMoveForward = !cursorMoveForward;
            }
        }
        else
        {
            if (cursorPos > 0)
            {
                cursorPos --;
            }
            else
            {
                cursorMoveForward = !cursorMoveForward;
            }
        }

        passwordTextField->SetCursorPos(cursorPos);
        cursorUpdateTime = 0.0f;
    }

    onScreenTime += timeElapsed;
    if(onScreenTime > INPUT_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }

    TestTemplate<InputTest>::Update(timeElapsed);
}

bool InputTest::RunTest(int32 testNum)
{
	TestTemplate<InputTest>::RunTest(testNum);
	return testFinished;
}


void InputTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	if (obj == testButton)
	{
		testFinished = true;
	}
	else if (obj == removeFromParentButton)
	{
		removeFromParentButton->RemoveFromParent();
	}
    else if (obj == disableInEventButton)
    {
        DVASSERT(!disableInEventButton->GetDisabled());
        disableInEventButton->SetStateText(0xFF, L"Disabled");
        disableInEventButton->SetDisabled(true);
    }
}

bool InputTest::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, WideString & replacementString)
{
	if (replacementLocation < 0 || replacementLength < 0)
	{
		staticText->SetText(L"");
		return true;
	}

	WideString resultString = textField->GetText();
	resultString.replace(replacementLocation, replacementLength, replacementString);
	staticText->SetText(resultString);

	return true;
}

void InputTest::OnPageLoaded(DAVA::BaseObject * caller, void * param, void *callerData)
{
	UIWebView* webView = dynamic_cast<UIWebView*>(caller);
	if(NULL == webView)
	{
		return;
	}
	
	webView->SetVisible(true);
}

void InputTest::DisplayUIControlsSize()
{
	Logger::Info("Sizeof UIControl is\t%i", sizeof(UIControl));
	Logger::Info("Sizeof UIMovieView is\t%i", sizeof(UIMovieView));
	Logger::Info("Sizeof UIScrollView is\t%i", sizeof(UIScrollView));
	Logger::Info("Sizeof UIAggregatorControl is\t%i", sizeof(UIAggregatorControl));
	Logger::Info("Sizeof UIWebView is\t%i", sizeof(UIWebView));
	Logger::Info("Sizeof UIButton is\t%i", sizeof(UIButton));
	Logger::Info("Sizeof UIControlBackground is\t%i", sizeof(UIControlBackground));
	Logger::Info("Sizeof UIList is\t%i", sizeof(UIList));
	Logger::Info("Sizeof UIPopup is\t%i", sizeof(UIPopup));
	Logger::Info("Sizeof UIScreen is\t%i", sizeof(UIScreen));
	Logger::Info("Sizeof UIScrollBar is\t%i", sizeof(UIScrollBar));
	Logger::Info("Sizeof UISlider is\t%i", sizeof(UISlider));
	Logger::Info("Sizeof UISpinner is\t%i", sizeof(UISpinner));
	Logger::Info("Sizeof UIStaticText is\t%i", sizeof(UIStaticText));
	Logger::Info("Sizeof UISwitch is\t%i", sizeof(UISwitch));
	Logger::Info("Sizeof UITextField is\t%i", sizeof(UITextField));
}
