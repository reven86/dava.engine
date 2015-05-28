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


#ifndef __UI_TEST_TEMPLATE__
#define __UI_TEST_TEMPLATE__

#include "DAVAEngine.h"
#include "TestTemplate.h"

using namespace DAVA;

template <class T>
class UITestTemplate: public TestTemplate<T>
{
    const static float32 TEST_TIME;
    
protected:
    ~UITestTemplate();

public:
    UITestTemplate(const String & screenName);

    virtual void LoadResources();
    virtual void UnloadResources();
    virtual bool RunTest(int32 testNum);

    virtual void DidAppear();
    virtual void Update(float32 timeElapsed);

private:

    UIButton* finishButton;

    bool testFinished;
    float testTime;

    void ButtonPressed(BaseObject *obj, void *data, void *callerData);
};


template <class T>
const float32 UITestTemplate<T>::TEST_TIME = 10.f;

template <class T>
UITestTemplate<T>::UITestTemplate(const String & screenName)
:    TestTemplate<T>(screenName)
,   testFinished(false)
,   testTime(0.f)
,   finishButton(NULL)
{
}

template <class T>
UITestTemplate<T>::~UITestTemplate()
{
    SafeRelease(finishButton);
}

template <class T>
void UITestTemplate<T>::LoadResources()
{
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(20);
    
    finishButton = new UIButton(Rect(0, 0, 300, 30));
    finishButton->SetStateFont(0xFF, font);
    finishButton->SetStateText(0xFF, L"Finish Test");
    finishButton->SetStateFontColor(0xFF, Color::White);
    finishButton->SetDebugDraw(true);
    finishButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UITestTemplate::ButtonPressed));
    TestTemplate<T>::AddControl(finishButton);
    
    font->Release();
}

template <class T>
void UITestTemplate<T>::UnloadResources()
{
    TestTemplate<T>::RemoveAllControls();
    SafeRelease(finishButton);
}

template <class T>
bool UITestTemplate<T>::RunTest(int32 testNum)
{
    TestTemplate<T>::RunTest(testNum);
    return testFinished;
}


template <class T>
void UITestTemplate<T>::DidAppear()
{
    testTime = 0.f;
}

template <class T>
void UITestTemplate<T>::Update(float32 timeElapsed)
{
    testTime += timeElapsed;
    if(testTime > TEST_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<T>::Update(timeElapsed);
}

template <class T>
void UITestTemplate<T>::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    testFinished = true;
}



#endif /* defined(__UI_TEST_TEMPLATE__) */
