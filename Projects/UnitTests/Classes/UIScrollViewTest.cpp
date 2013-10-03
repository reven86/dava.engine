//
//  UIScrollViewTest.cpp
//  TemplateProjectMacOS
//
//  Created by Denis Bespalov on 5/20/13.
//
//

#include "UIScrollViewTest.h"

static const float LIST_TEST_AUTO_CLOSE_TIME = 30.0f;

UIScrollViewTest::UIScrollViewTest() :
 TestTemplate<UIScrollViewTest>("UIScrollViewTest")
{
	testFinished = false;
	
	RegisterFunction(this, &UIScrollViewTest::TestFunction, Format("UIScrollViewTest"), NULL);
	
	onScreenTime = 0.f;
}

void UIScrollViewTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(14);
    font->SetColor(Color::White());

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
        horizontalScrollbar->SetDelegate(scrollView);
    }
	
    control = FindByName("VerticalScrollbar");
    if( control )
    {
        UIScrollBar *verticalScrollbar = DynamicTypeCheck<UIScrollBar *>( control );
        verticalScrollbar->GetSlider()->SetSprite("~res:/Gfx/UI/VerticalScroll", 0);
        verticalScrollbar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_VERTICAL);
        verticalScrollbar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);
        verticalScrollbar->SetOrientation( UIScrollBar::ORIENTATION_VERTICAL );
        verticalScrollbar->SetDelegate(scrollView);
    }

	UIControl *testControlChild = new UIControl(Rect(100, 100, 150, 150));
	testControlChild->SetDebugDraw(true);
	testControlChild->GetBackground()->SetColor(Color(0.3333, 0.3333, 0.5555, 1.0000));
	testControlChild->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControlChild->SetName("CONTROL_3");
	testControlChild->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	
	UIControl *testControl = new UIControl(Rect(50, 0, 150, 150));
	testControl->SetDebugDraw(true);
	testControl->GetBackground()->SetColor(Color(0.3333, 0.6667, 0.4980, 1.0000));
	testControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testControl->SetName("CONTROL_2");
	testControl->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	testControl->AddControl(testControlChild);
	
	UIButton *testButton = new UIButton(Rect(10, 50, 250, 100));
	testButton->SetDebugDraw(true);
	testButton->SetStateFont(STATE_NORMAL, font);
	testButton->SetStateText(STATE_NORMAL, L"First button");
	testButton->GetBackground()->SetColor(Color(0.6667, 0.6667, 0.4980, 1.0000));
	testButton->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	testButton->SetName("CONTROL_1");
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	testButton->AddControl(testControl);
	
	scrollView->AddControlToContainer(testButton);
	
	testMessageText = new UIStaticText(Rect(10, 210, 300, 30));
	testMessageText->SetFont(font);
	AddControl(testMessageText);
	
	finishTestBtn = new UIButton(Rect(10, 260, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
	finishTestBtn->SetStateText(0xFF, L"Finish test");

	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIScrollViewTest::ButtonPressed));
	AddControl(finishTestBtn);
}

void UIScrollViewTest::UnloadResources()
{
	RemoveAllControls();
	SafeRelease(finishTestBtn);
	SafeRelease(testMessageText);
}

void UIScrollViewTest::DidAppear()
{
    onScreenTime = 0.f;
}

void UIScrollViewTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > LIST_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<UIScrollViewTest>::Update(timeElapsed);
}

void UIScrollViewTest::TestFunction(PerfFuncData * data)
{
	return;
}

bool UIScrollViewTest::RunTest(int32 testNum)
{
	TestTemplate<UIScrollViewTest>::RunTest(testNum);
	return testFinished;
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

	if (obj == finishTestBtn)
	{
		testFinished = true;
	}
}
