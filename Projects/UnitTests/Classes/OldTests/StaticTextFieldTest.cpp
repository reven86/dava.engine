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


#include "StaticTextFieldTest.h"

using namespace DAVA;

namespace
{
    const float autoCloseTime{ 300.0f };
}

StaticTextFieldTest::StaticTextFieldTest()
    : TestTemplate<StaticTextFieldTest>("StaticTextFieldTest"),
    finishTestButton(nullptr),
    setStaticButton(nullptr),
    setNormalButton(nullptr),
    add10ToAlfaButton(nullptr),
    setVisibleButton(nullptr),
    setHideButton(nullptr),
    overlapedImage(nullptr),
    uiTextField1(nullptr),
    uiTextField2(nullptr),
    testFinished(false),
    onScreenTime(0.f)
{
    RegisterFunction(this, &StaticTextFieldTest::TestFunc,
        Format("StaticTextFieldTest"), nullptr);
}

void StaticTextFieldTest::LoadResources()
{
    uiTextField1 = new UITextField(Rect(300, 100, 400, 40));
    uiTextField1->SetVisible(true);
    uiTextField1->SetDebugDraw(true);
    // only http://www.microsoft.com works with IE ole component nice
    uiTextField1->SetText(L"start text ...some test text goes here... end");

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(30);

#ifdef __DAVAENGINE_IPHONE__
#else
	uiTextField1->SetFont(font);
#endif
    uiTextField1->SetTextColor(Color::White);

    uiTextField1->SetSpriteAlign(ALIGN_RIGHT);
	uiTextField1->SetTextAlign(ALIGN_LEFT | ALIGN_BOTTOM);
	uiTextField1->SetDebugDraw(true);
	uiTextField1->SetDelegate(new UITextFieldDelegate());
	uiTextField1->SetIsPassword(false);

    AddControl(uiTextField1);

    overlapedImage = new UIControl(Rect(500, 0, 300, 300));
    overlapedImage->SetSprite("~res:/Gfx/UI/Rotation", 0);
    overlapedImage->SetDebugDraw(true);
    AddControl(overlapedImage);

    const float32 w = 40;

    CreateUIButton(finishTestButton, font, Rect(0, 510, 300, w),
        L"Finish Test", &StaticTextFieldTest::OnButtonPressed);

    CreateUIButton(add10ToAlfaButton, font, Rect(0 + 300 * 1, 510 + w, 300, w),
        L"+10 to Alfa", &StaticTextFieldTest::OnButtonAdd10ToAlfa);

    CreateUIButton(minus10FromAlfaButton, font,
        Rect(0 + 300 * 2, 510 + w, 300, w),
        L"-10 to Alfa", &StaticTextFieldTest::OnButtonMinus10FromAlfa);

    CreateUIButton(setVisibleButton, font,
        Rect(0 + 300 * 1, 510 + w * 4, 300, w), L"Show",
        &StaticTextFieldTest::OnButtonVisible);

    CreateUIButton(setHideButton, font,
        Rect(0 + 300 * 2, 510 + w * 4, 300, w), L"Hide",
        &StaticTextFieldTest::OnButtonHide);

    SafeRelease(font);
}

void StaticTextFieldTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(uiTextField1);
    SafeRelease(uiTextField2);
    SafeRelease(finishTestButton);
    SafeRelease(setStaticButton);
    SafeRelease(setNormalButton);
    SafeRelease(add10ToAlfaButton);
    SafeRelease(minus10FromAlfaButton);
    SafeRelease(setVisibleButton);
    SafeRelease(setHideButton);
}

bool StaticTextFieldTest::RunTest(int32 testNum)
{
    TestTemplate<StaticTextFieldTest>::RunTest(testNum);
    return testFinished;
}

void StaticTextFieldTest::DidAppear()
{
    onScreenTime = 0.f;
}

void StaticTextFieldTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if (onScreenTime > autoCloseTime)
    {
        testFinished = true;
    }

    TestTemplate<StaticTextFieldTest>::Update(timeElapsed);
}

void StaticTextFieldTest::TestFunc(PerfFuncData *)
{
    return;
}

void StaticTextFieldTest::OnButtonPressed(BaseObject *, void *, void *)
{
    testFinished = true;
}

void StaticTextFieldTest::OnButtonAdd10ToAlfa(BaseObject *obj, void *data,
    void *callerData)
{
    Sprite* spr = uiTextField1->GetSprite();
    if (nullptr != spr)
    {
        UIControlBackground* back = uiTextField1->GetBackground();
        auto color = back->GetColor();
        color.a = Min(1.0f, color.a + 0.1f);
        back->SetColor(color);
    }

    auto col = uiTextField1->GetTextColor();
    col.a = Min(1.0f, col.a + 0.1f);

    uiTextField1->SetTextColor(col);
}

void StaticTextFieldTest::OnButtonMinus10FromAlfa(BaseObject *obj, void *data,
    void *callerData)
{
    Sprite* spr = uiTextField1->GetSprite();
    if (spr)
    {
        UIControlBackground* back = uiTextField1->GetBackground();
        Color color = back->GetColor();
        color.a = Max(0.f, color.a - 0.1f);
        back->SetColor(color);
    }

    auto col = uiTextField1->GetTextColor();
    col.a = Max(0.f, col.a - 0.1f);

    uiTextField1->SetTextColor(col);
}

void StaticTextFieldTest::CreateUIButton(UIButton*& button, Font * font,
    const Rect& rect, const WideString& str,
    void (StaticTextFieldTest::*targetFunction)(BaseObject*, void*, void*))
{
    button = new UIButton(rect);
    button->SetStateFont(0xFF, font);
    button->SetStateText(0xFF, str);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetDebugDraw(true);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE,
        Message(this, targetFunction));
    AddControl(button);
}



void StaticTextFieldTest::OnButtonVisible(BaseObject*, void*, void*)
{
    uiTextField1->SetVisible(true);
}

void StaticTextFieldTest::OnButtonHide(BaseObject*, void*, void*)
{
    uiTextField1->SetVisible(false);
}

