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


#include "Tests/TextFieldTest.h"

using namespace DAVA;

namespace
{

class InputDelegate : public UITextFieldDelegate
{
public:
    void TextFieldOnTextChanged(UITextField * textField, const WideString& newText, const WideString& oldText) override
    {
    }
    
};

}

TextFieldTest::TextFieldTest()
: BaseScreen("TextFieldTest")
{
}

void TextFieldTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    ScopedPtr<FTFont> bigFont(FTFont::Create("~res:/Fonts/korinna.ttf"));
    bigFont->SetSize(24.f);
    
    InputDelegate * d = new InputDelegate;
    
    UITextField * field = new UITextField(Rect(0, 70, 200, 50));
    field->SetFont(font);
    field->SetDebugDraw(true);
    field->SetText(L"Test text inside UITextField used for test");
    field->SetDelegate(d);
    AddControl(field);
    SafeRelease(field);
    
    field = new UITextField(Rect(0, 130, 200, 50));
    field->SetFont(font);
    field->SetFocused();
    field->SetDebugDraw(true);
    field->SetText(L"Test text inside UITextField used for test");
    field->SetDelegate(d);
    
    AddControl(field);
    SafeRelease(field);


    UIButton * button = new UIButton(Rect(0,0, 200, 50));
    button->SetStateFont(0xFF, font);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetStateText(0xFF, L"Show/Hide");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &TextFieldTest::OnShowHideClick));
    AddControl(button);
    SafeRelease(button);
    

    topLayerControl = new UIControl(Rect(50, 0, 50, 200));
    topLayerControl->GetBackground()->SetColor(Color(1.0f, 0.0f, 0.0f, 0.5f));
    topLayerControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    topLayerControl->GetBackground()->SetColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    topLayerControl->SetDebugDraw(true);
    AddControl(topLayerControl);
}

void TextFieldTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    
    SafeRelease(topLayerControl);
}



void TextFieldTest::OnShowHideClick(BaseObject* sender, void * data, void * callerData)
{
    if (nullptr != topLayerControl)
    {
        static bool isVisible = true;
        topLayerControl->SetVisible(isVisible);
        isVisible = !isVisible;
    }

}
