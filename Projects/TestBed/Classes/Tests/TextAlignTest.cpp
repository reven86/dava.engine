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


#include "Tests/TextAlignTest.h"

using namespace DAVA;

class InputDelegate : public UITextFieldDelegate
{
public:
    InputDelegate(UIStaticText* text)
    {
        controlText = SafeRetain(text);
    }

    ~InputDelegate()
    {
        SafeRelease(controlText);
    }

    void TextFieldOnTextChanged(UITextField * textField, const WideString& newText, const WideString& oldText) override
    {
        controlText->SetText(newText);
    }

private:
    UIStaticText* controlText;
};

static const Color RED = Color(1.f, 0.f, 0.f, 1.f);
static const Color GREEN = Color(0.f, 1.f, 0.f, 1.f);

TextAlignTest::TextAlignTest()
: BaseScreen("TextAlignTest")
, previewText(nullptr)
, inputText(nullptr)
, inputDelegate(nullptr)
, topLeftButton(nullptr)
, topCenterButton(nullptr)
, topRightButton(nullptr)
, middleLeftButton(nullptr)
, middleCenterButton(nullptr)
, middleRightButton(nullptr)
, bottomLeftButton(nullptr)
, bottomCenterButton(nullptr)
, bottomRightButton(nullptr)
{
}

TextAlignTest::~TextAlignTest()
{
}

void TextAlignTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    ScopedPtr<UIStaticText> previewLabel(new UIStaticText(Rect(5, 5, 400, 20)));
    previewLabel->SetFont(font);
    previewLabel->SetTextColor(Color::White);
    previewLabel->SetText(L"Preview:");
    previewLabel->SetTextAlign(ALIGN_LEFT);
    AddControl(previewLabel);
    
    previewText = new UIStaticText(Rect(5, 30, 400, 200));
    previewText->SetFont(font);
    previewText->SetTextColor(Color::White);
    previewText->SetText(L"");
    previewText->SetDebugDraw(true);
    previewText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(previewText);

    ScopedPtr<UIStaticText> inputLabel(new UIStaticText(Rect(5, 235, 400, 20)));
    inputLabel->SetFont(font);
    inputLabel->SetTextColor(Color::White);
    inputLabel->SetText(L"Input:");
    inputLabel->SetTextAlign(ALIGN_LEFT);
    AddControl(inputLabel);

    inputText = new UITextField(Rect(5, 260, 400, 200));
    inputText->SetFont(font);
    inputText->SetTextColor(Color::White);
    inputText->SetText(L"");
    inputText->SetDebugDraw(true);
    inputText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    inputText->SetDelegate(inputDelegate = new InputDelegate(previewText));
    AddControl(inputText);

    ScopedPtr<UIStaticText> alignsLabel(new UIStaticText(Rect(450, 5, 200, 20)));
    alignsLabel->SetFont(font);
    alignsLabel->SetTextColor(Color::White);
    alignsLabel->SetText(L"Aligns:");
    alignsLabel->SetTextAlign(ALIGN_LEFT);
    AddControl(alignsLabel);

    topLeftButton = new UIButton(Rect(450, 30, 60, 20));
    topLeftButton->SetStateFont(0xFF, font);
    topLeftButton->SetStateFontColor(0xFF, Color::White);
    topLeftButton->SetStateText(0xFF, L"Top");
    topLeftButton->SetDebugDraw(true);
    topLeftButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    topLeftButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(topLeftButton);

    topCenterButton = new UIButton(Rect(520, 30, 60, 20));
    topCenterButton->SetStateFont(0xFF, font);
    topCenterButton->SetStateFontColor(0xFF, Color::White);
    topCenterButton->SetStateText(0xFF, L"Middle");
    topCenterButton->SetDebugDraw(true);
    topCenterButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    topCenterButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(topCenterButton);

    topRightButton = new UIButton(Rect(590, 30, 60, 20));
    topRightButton->SetStateFont(0xFF, font);
    topRightButton->SetStateFontColor(0xFF, Color::White);
    topRightButton->SetStateText(0xFF, L"Bottom");
    topRightButton->SetDebugDraw(true);
    topRightButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    topRightButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(topRightButton);

    middleLeftButton = new UIButton(Rect(450, 55, 60, 20));
    middleLeftButton->SetStateFont(0xFF, font);
    middleLeftButton->SetStateFontColor(0xFF, Color::White);
    middleLeftButton->SetStateText(0xFF, L"Left");
    middleLeftButton->SetDebugDraw(true);
    middleLeftButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    middleLeftButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(middleLeftButton);

    middleCenterButton = new UIButton(Rect(520, 55, 60, 20));
    middleCenterButton->SetStateFont(0xFF, font);
    middleCenterButton->SetStateFontColor(0xFF, Color::White);
    middleCenterButton->SetStateText(0xFF, L"Center");
    middleCenterButton->SetDebugDraw(true);
    middleCenterButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    middleCenterButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(middleCenterButton);

    middleRightButton = new UIButton(Rect(590, 55, 60, 20));
    middleRightButton->SetStateFont(0xFF, font);
    middleRightButton->SetStateFontColor(0xFF, Color::White);
    middleRightButton->SetStateText(0xFF, L"Right");
    middleRightButton->SetDebugDraw(true);
    middleRightButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    middleRightButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(middleRightButton);

    middleLeftButton = new UIButton(Rect(450, 55, 60, 20));
    middleLeftButton->SetStateFont(0xFF, font);
    middleLeftButton->SetStateFontColor(0xFF, Color::White);
    middleLeftButton->SetStateText(0xFF, L"Left");
    middleLeftButton->SetDebugDraw(true);
    middleLeftButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    middleLeftButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(middleLeftButton);

    middleCenterButton = new UIButton(Rect(520, 55, 60, 20));
    middleCenterButton->SetStateFont(0xFF, font);
    middleCenterButton->SetStateFontColor(0xFF, Color::White);
    middleCenterButton->SetStateText(0xFF, L"Center");
    middleCenterButton->SetDebugDraw(true);
    middleCenterButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    middleCenterButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(middleCenterButton);

    middleRightButton = new UIButton(Rect(590, 55, 60, 20));
    middleRightButton->SetStateFont(0xFF, font);
    middleRightButton->SetStateFontColor(0xFF, Color::White);
    middleRightButton->SetStateText(0xFF, L"Right");
    middleRightButton->SetDebugDraw(true);
    middleRightButton->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextAlignTest::OnAlignButtonClick));
    middleRightButton->SetStateTextAlign(0xFF, ALIGN_TOP | ALIGN_LEFT);
    AddControl(middleRightButton);

    SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
}

void TextAlignTest::UnloadResources()
{
    SafeRelease(previewText);
    SafeRelease(inputText);
    SafeDelete(inputDelegate);
    SafeRelease(topLeftButton);
    SafeRelease(topCenterButton);
    SafeRelease(topRightButton);
    SafeRelease(middleLeftButton);
    SafeRelease(middleCenterButton);
    SafeRelease(middleRightButton);
}

void TextAlignTest::SetPreviewAlign(DAVA::int32 align)
{
    previewText->SetTextAlign(align);
    topLeftButton->SetDebugDrawColor(align & (ALIGN_TOP | ALIGN_LEFT) ? GREEN : RED);
    topCenterButton->SetDebugDrawColor(align & (ALIGN_TOP | ALIGN_HCENTER) ? GREEN : RED);
    middleRightButton->SetDebugDrawColor(align & (ALIGN_TOP | ALIGN_RIGHT) ? GREEN : RED);
    middleLeftButton->SetDebugDrawColor(align & (ALIGN_VCENTER | ALIGN_LEFT) ? GREEN : RED);
    middleCenterButton->SetDebugDrawColor(align & (ALIGN_VCENTER | ALIGN_HCENTER) ? GREEN : RED);
    middleRightButton->SetDebugDrawColor(align & (ALIGN_VCENTER | ALIGN_RIGHT) ? GREEN : RED);
    bottomLeftButton->SetDebugDrawColor(align & (ALIGN_BOTTOM | ALIGN_LEFT) ? GREEN : RED);
    bottomCenterButton->SetDebugDrawColor(align & (ALIGN_BOTTOM | ALIGN_HCENTER) ? GREEN : RED);
    bottomRightButton->SetDebugDrawColor(align & (ALIGN_BOTTOM | ALIGN_RIGHT) ? GREEN : RED);
}

void TextAlignTest::OnAlignButtonClick(BaseObject* sender, void * data, void * callerData)
{
    if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topCenterButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topRightButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    else if (sender == topLeftButton)
    {
        SetPreviewAlign(ALIGN_TOP | ALIGN_LEFT);
    }
    
}
