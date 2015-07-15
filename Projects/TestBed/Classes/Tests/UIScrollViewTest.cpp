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

#include "Tests/UIScrollViewTest.h"

using namespace DAVA;

UIScrollViewTest::UIScrollViewTest()
    : BaseScreen("UIScrollViewTest")
{
}

void UIScrollViewTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(14);

    UIYamlLoader::Load( this, "~res:/UI/Test/ScrollScreen.yaml" );
	scrollView = DynamicTypeCheck<UIScrollView *>( FindByName( "Scrollview" ) );
	
	UIControl* innerControl = FindByName("UIControl1");
	if (innerControl)
	{
		innerControl->SetSprite("~res:/Gfx/UI/HorizontalScroll", 0);
		innerControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
	}
	
    UIControl *control = FindByName("HorizontalScrollbar");
    if( control )
    {
        UIScrollBar *horizontalScrollbar = DynamicTypeCheck<UIScrollBar *>( control );
        horizontalScrollbar->GetSlider()->SetSprite("~res:/Gfx/UI/HorizontalScroll", 0);
        horizontalScrollbar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
		horizontalScrollbar->GetSlider()->GetBackground()->SetLeftRightStretchCap(10);
        horizontalScrollbar->SetOrientation( UIScrollBar::ORIENTATION_HORIZONTAL );
    }
	
    control = FindByName("VerticalScrollbar");
    if( control )
    {
        UIScrollBar *verticalScrollbar = DynamicTypeCheck<UIScrollBar *>( control );
        verticalScrollbar->GetSlider()->SetSprite("~res:/Gfx/UI/VerticalScroll", 0);
        verticalScrollbar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_VERTICAL);
        verticalScrollbar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);
        verticalScrollbar->SetOrientation( UIScrollBar::ORIENTATION_VERTICAL );
    }
	
	UIControl *testControl4 = new UIControl(Rect(1200.f, 1400.f, 250.f, 250.f));
	testControl4->SetDebugDraw(true);
	testControl4->GetBackground()->SetColor(Color(0.3333f, 0.3333f, 0.5555f, 1.0000f));
	testControl4->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl4->SetName("CONTROL_4");
	testControl4->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	scrollView->AddControlToContainer(testControl4);
	SafeRelease(testControl4);

	UIControl *testControlChild = new UIControl(Rect(100.f, 100.f, 150.f, 150.f));
	testControlChild->SetDebugDraw(true);
	testControlChild->GetBackground()->SetColor(Color(0.3333f, 0.3333f, 0.5555f, 1.0000f));
	testControlChild->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControlChild->SetName("CONTROL_3");
	testControlChild->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	
	UIControl *testControl = new UIControl(Rect(50.f, 0.f, 150.f, 150.f));
	testControl->SetDebugDraw(true);
	testControl->GetBackground()->SetColor(Color(0.3333f, 0.6667f, 0.4980f, 1.0000f));
	testControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl->SetName("CONTROL_2");
	testControl->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	testControl->AddControl(testControlChild);
	
	UIButton *testButton = new UIButton(Rect(10.f, 50.f, 250.f, 100.f));
	testButton->SetDebugDraw(true);
	testButton->SetStateFont(STATE_NORMAL, font);
	testButton->SetStateFontColor(STATE_NORMAL, Color::White);
	testButton->SetStateText(STATE_NORMAL, L"First button");
	testButton->GetBackground()->SetColor(Color(0.6667f, 0.6667f, 0.4980f, 1.0000f));
	testButton->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testButton->SetName("CONTROL_1");
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	testButton->AddControl(testControl);
	
	scrollView->AddControlToContainer(testButton);
	// Check how controls with negative coodinates are pushed inside scrollContainer
	UIControl *testControl5 = new UIControl(Rect(-50.f, 500.f, 150.f, 150.f));
	testControl5->SetDebugDraw(true);
	testControl5->GetBackground()->SetColor(Color(0.3333f, 0.3333f, 0.5555f, 1.0000f));
	testControl5->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl5->SetName("CONTROL_5");

	UIControl *testControl6 = new UIControl(Rect(-50.f, -50.f, 25.f, 25.f));
	testControl6->SetDebugDraw(true);
	testControl6->GetBackground()->SetColor(Color(0.3333f, 0.6667f, 0.4980f, 1.0000f));
	testControl6->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl6->SetName("CONTROL_6");

	testControl5->AddControl(testControl6);
	
	UIControl *testControl7 = new UIControl(Rect(-100.f, 15.f, 35.f, 35.f));
	testControl7->SetDebugDraw(true);
	testControl7->GetBackground()->SetColor(Color(0.6667f, 0.3333f, 0.7880f, 1.0000f));
	testControl7->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl7->SetName("CONTROL_7");

	testControl6->AddControl(testControl7);
	SafeRelease(testControl6);
	SafeRelease(testControl7);
	
	UIControl *testControl8 = new UIControl(Rect(-70.f, 50.f, 25.f, 25.f));
	testControl8->SetDebugDraw(true);
	testControl8->GetBackground()->SetColor(Color(0.6667f, 0.3333f, 0.4980f, 1.0000f));
	testControl8->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl8->SetName("CONTROL_8");

	testControl5->AddControl(testControl8);
	SafeRelease(testControl8);

	scrollView->AddControlToContainer(testControl5);
	scrollView->RecalculateContentSize();
	SafeRelease(testControl5);

	SafeRelease(testControlChild);
	SafeRelease(testControl);
	SafeRelease(testButton);
	
	testMessageText = new UIStaticText(Rect(10, 10, 300, 30));
	testMessageText->SetFont(font);
	testMessageText->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
	testMessageText->GetBackground()->SetColor(Color(0.5, 0.0, 0.25, 1.0));
	testMessageText->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	AddControl(testMessageText);
	
	finishTestBtn = new UIButton(Rect(10, 310, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
	finishTestBtn->SetStateFontColor(0xFF, Color(1.0, 0.0, 0.0, 0.75));
	finishTestBtn->SetStateText(0xFF, L"Finish test");

	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	AddControl(finishTestBtn);

	SafeRelease(font);
    
    BaseScreen::LoadResources();
}

void UIScrollViewTest::UnloadResources()
{
	RemoveAllControls();
	SafeRelease(finishTestBtn);
	SafeRelease(testMessageText);
    
    BaseScreen::UnloadResources();
}

void UIScrollViewTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	UIControl *control = dynamic_cast<UIControl*>(obj);
	if (control)
	{
		String msg = Format("Tap on control - %s", control->GetName().c_str());
		testMessageText->SetText(StringToWString(msg));
	}
	else
	{
		testMessageText->SetText(L"");
	}
}
