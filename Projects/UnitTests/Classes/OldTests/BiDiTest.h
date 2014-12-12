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

#ifndef __BIDITEST_H__
#define __BIDITEST_H__

#include "DAVAEngine.h"
#include "TestTemplate.h"

using namespace DAVA;

struct sBiDiTestData
{
	sBiDiTestData(const WideString& text, int32 align, bool useRtl, int32 visualAlign, bool isRtl)
	: text(text)
	, align(align)
	, useRtl(useRtl)
	, visualAlign(visualAlign)
	, isRtl(isRtl)
	{}
	WideString text;
	int32 align;
	bool useRtl;
	int32 visualAlign;
	bool isRtl;
};

class BiDiTest: public TestTemplate<BiDiTest>, public UITextFieldDelegate
{
protected:
    ~BiDiTest(){}
public:
	BiDiTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);

	virtual void Update(float32 timeElapsed);

	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);

	void TestFunction(PerfFuncData * data);
	void ManualTestStartFunction(PerfFuncData * data);
	void ManualTestProcessFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);

	UIStaticText* staticText;
	UIStaticText* autoStaticText;
	UITextField* textField;
	
    UIButton* modeButton;
	UIButton* rtlButton;
	UIButton* testButton;
	
	void* delegate;

	bool testFinished;
	bool manualStarted;
	float onScreenTime;
    
};

#endif 