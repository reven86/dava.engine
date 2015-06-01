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


#ifndef __ALIGN_TEST_H__
#define __ALIGN_TEST_H__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"
#include "Render/RenderManager.h"

class AlignTest : public TestTemplate<AlignTest>, public ScreenShotCallbackDelegate
{
protected:
    ~AlignTest(){}
public:
	AlignTest();
    
	virtual void LoadResources();
	virtual void UnloadResources();

	virtual void MultilineEnable(PerfFuncData * testData);
	virtual void ResizeControl(PerfFuncData * testData);
	virtual void MoveControl(PerfFuncData * testData);
	virtual void AlignText(PerfFuncData * testData);
    
private:
    UIStaticText * staticText;
    UIStaticText * staticText2;
	
	void OnScreenShot(Image *testImage);
	void MakeScreenShot();
	void VerifyTestImage(Image *testImage);
	
	PerfFuncData * data;
	int currenTestIndex;
	int currentAlignIndex;
	int GetAlignTypesCount();
    int GetAlignType(int index);
    const static int alignTypesData[];
};

#endif /* defined(__ALIGN_TEST_H__) */
