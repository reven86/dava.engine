//
//  UIListTest.cpp
//  TemplateProjectMacOS
//
//  Created by Denis Bespalov on 4/9/13.
//
//

#include "UIListTest.h"

static const int32 CELL_COUNT = 50;
static const float32 LIST_TEST_AUTO_CLOSE_TIME = 300.0f;

UIListTestDelegate::UIListTestDelegate(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
	: UIControl(rect, rectInAbsoluteCoordinates)
{
	cellSize = Vector2(100.0f, 30.0f);
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
        cell = new UIListCell(Rect(0, 0, (float32)list->size.x, (float32)cellSize.y), "List cell", list->GetAggregatorPath());
		
		/*Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    	DVASSERT(font);
		font->SetSize(20);
    	font->SetColor(Color::White());
		
		cell->SetStateFont(UIControl::STATE_NORMAL, font);
		cell->SetStateText(UIControl::STATE_NORMAL, StringToWString(Format("Cell %d",index)));*/
		//cell->SetInputEnabled(true);
    }
	   
    return cell;
}

int32 UIListTestDelegate::CellHeight(UIList *list, int32 index)
{
   	return (int32)cellSize.y;
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
    font->SetColor(Color::White());

	YamlParser * parser = YamlParser::Create("~res:/TestData/ListTest/ListData.yaml");
	UIYamlLoader * loader = new UIYamlLoader();

	bool scrollBarAddedToFirstList = false;

	if (parser && parser->GetRootNode())
	{
		for (MultiMap<String, YamlNode*>::iterator t = parser->GetRootNode()->AsMap().begin(); t != parser->GetRootNode()->AsMap().end(); ++t)
		{
			YamlNode * listNode = t->second;
			// Skip empty list node
			if (!listNode) continue;
		
			UIList *list = NULL;
			list = new UIList();
			list->SetDebugDraw(true);
			list->LoadFromYamlNode(listNode, loader);
			
			// Set delegate for list
			UIListTestDelegate *listDelegate = NULL;
			listDelegate = new UIListTestDelegate(list->GetRect());
			list->SetDelegate(listDelegate);
			
			AddControl(list);
			
			if (!scrollBarAddedToFirstList)
			{
				UIScrollBar *verticalScrollbar = new UIScrollBar(Rect(940, 0, 20, 620));
				verticalScrollbar->GetSlider()->SetSprite("~res:/Gfx/UI/VerticalScroll", 0);
				verticalScrollbar->GetSlider()->GetBackground()->SetDrawType(UIControlBackground::DRAW_STRETCH_VERTICAL);
				verticalScrollbar->GetSlider()->GetBackground()->SetTopBottomStretchCap(10);
				verticalScrollbar->SetOrientation( UIScrollBar::ORIENTATION_VERTICAL );
				verticalScrollbar->SetDelegate(list);
				AddControl(verticalScrollbar);

				scrollBarAddedToFirstList = true;
			}
		}
	}

	SafeRelease(loader);
	SafeRelease(parser);

	finishTestBtn = new UIButton(Rect(10, 210, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
	finishTestBtn->SetStateText(0xFF, L"Finish test");

	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIListTest::ButtonPressed));
	AddControl(finishTestBtn);
}

void UIListTest::UnloadResources()
{
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