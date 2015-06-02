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
//  UIListTest.cpp
//  TemplateProjectMacOS
//
//  Created by Denis Bespalov on 4/9/13.
//
//

#include "UIListTest.h"

static const int32 CELL_COUNT = 50;
static const float32 LIST_TEST_AUTO_CLOSE_TIME = 30.0f;

UIListTestDelegate::UIListTestDelegate(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
	: UIControl(rect, rectInAbsoluteCoordinates)
{
	cellSize = Vector2(100.0f, 40.0f);
}

UIListTestDelegate::~UIListTestDelegate()
{
}
	
int32 UIListTestDelegate::ElementsCount(UIList *list)
{
	return CELL_COUNT;
}
	
UIListCell *UIListTestDelegate::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *cell = list->GetReusableCell("List cell"); //try to get cell from the reusable cells store
	
    if(!cell)
    { //if cell of requested type isn't find in the store create new cell
        cell = new UIListCell(Rect(0, 0, cellSize.x, cellSize.y), "List cell", list->GetAggregatorPath());
		
		/*Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    	DVASSERT(font);
		font->SetSize(20);
    	font->SetColor(Color::White);
		
		cell->SetStateFont(UIControl::STATE_NORMAL, font);
		cell->SetStateText(UIControl::STATE_NORMAL, StringToWString(Format("Cell %d",index)));*/
		//cell->SetInputEnabled(true);
    }
	   
    return cell;
}

float32 UIListTestDelegate::CellWidth(UIList*, int32)
{
   	return cellSize.x;
}

float32 UIListTestDelegate::CellHeight(UIList*, int32)
{
   	return cellSize.y;
}

UIListTest::UIListTest() :
 TestTemplate<UIListTest>("UIListTest")
{
	testFinished = false;
	
	RegisterFunction(this, &UIListTest::TestFunction, Format("UIListTest"), NULL);
	
	onScreenTime = 0.f;
}

void UIListTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(14);

    UIYamlLoader::Load(this, "~res:/TestData/ListTest/ListData.yaml");
    
	bool scrollBarAddedToFirstList = false;
    const uint32 controlsCount = (uint32)GetChildren().size();
    for(uint32 i = 0; i < controlsCount; ++i)
    {
        UIList *list = dynamic_cast<UIList *>(FindByName(Format("UIList%d", i + 1)));
        if(!list)
        {
            continue;
        }
        
        // Set delegate for list
        UIListTestDelegate *listDelegate = new UIListTestDelegate(list->GetRect());
        list->SetDelegate(listDelegate);
        
        if (!scrollBarAddedToFirstList)
        {
            UIScrollBar *verticalScrollbar = new UIScrollBar(Rect(940, 0, 20, 620));
            verticalScrollbar->GetSlider()->SetSprite("~res:/Gfx/UI/VerticalScroll", 0);
            verticalScrollbar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_VERTICAL);
            verticalScrollbar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);
            verticalScrollbar->SetOrientation( UIScrollBar::ORIENTATION_VERTICAL );
            verticalScrollbar->SetDelegate(list);
            AddControl(verticalScrollbar);
            SafeRelease(verticalScrollbar);
            
            scrollBarAddedToFirstList = true;
        }
    }
    

	finishTestBtn = new UIButton(Rect(10, 250, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
    finishTestBtn->SetStateFontColor(0xFF, Color::White);
	finishTestBtn->SetStateText(0xFF, L"Finish test");

	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIListTest::ButtonPressed));
	AddControl(finishTestBtn);

	SafeRelease(font);
}

void UIListTest::UnloadResources()
{
    const uint32 controlsCount = (uint32)GetChildren().size();
    for(uint32 i = 0; i < controlsCount; ++i)
    {
        UIList *list = dynamic_cast<UIList *>(FindByName(Format("UIList%d", i + 1)));
        if(!list)
        {
            continue;
        }
        
        // Set delegate for list
        UIListTestDelegate * delegate = (UIListTestDelegate *)list->GetDelegate();
        SafeRelease(delegate);
        list->SetDelegate(NULL);
    }
    
    
	RemoveAllControls();
	SafeRelease(finishTestBtn);
}

void UIListTest::DidAppear()
{
    onScreenTime = 0.f;
}

void UIListTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > LIST_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<UIListTest>::Update(timeElapsed);
}

void UIListTest::TestFunction(PerfFuncData * data)
{
	return;
}

bool UIListTest::RunTest(int32 testNum)
{
	TestTemplate<UIListTest>::RunTest(testNum);
	return testFinished;
}


void UIListTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	if (obj == finishTestBtn)
	{
		testFinished = true;
	}
}
