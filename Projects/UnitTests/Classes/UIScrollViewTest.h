//
//  UIScrollViewTest.h
//  TemplateProjectMacOS
//
//  Created by Denis Bespalov on 5/20/13.
//
//

#ifndef __UISCROLL_VIEW_TEST__
#define __UISCROLL_VIEW_TEST__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class UIScrollViewTest: public TestTemplate<UIScrollViewTest>
{

public:
	UIScrollViewTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	virtual void DidAppear();	
	virtual void Update(float32 timeElapsed);
	
	void TestFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UIButton* 		finishTestBtn;
	UIStaticText*	testMessageText;
	UIScrollView*	scrollView;
	bool 			testFinished;
		
	float32 onScreenTime;
};

#endif /* defined(__UISCROLL_VIEW_TEST__) */
