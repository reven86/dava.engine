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


#include "Tests/MultilineTest.h"

using namespace DAVA;

class TextDelegate1 : public UITextFieldDelegate
{
public:
    virtual ~TextDelegate1() = default;
    bool TextFieldKeyPressed(UITextField* /*textField*/, int32 replacementLocation, int32 replacementLength, WideString& replacementString) override
    {
        // Allow only small caps letters
        bool accept = true;
        for (size_t i = 0, n = replacementString.length();i < n && accept;++i)
        {
            accept = 'a' <= replacementString[i] && replacementString[i] <= 'z';
        }
        Logger::Debug("****** TextDelegate2::TextFieldKeyPressed: accepted=%d", accept);
        return accept;
    }
    void TextFieldOnTextChanged(UITextField* /*textField*/, const WideString& newText, const WideString& oldText) override
    {
        Logger::Debug("****** TextDelegate1::TextFieldOnTextChanged: new=%s, old=%s", WStringToString(newText).c_str(), WStringToString(oldText).c_str());
    }
    void TextFieldShouldReturn(UITextField* textField)
    {
        textField->CloseKeyboard();
    }
};

class TextDelegate2 : public UITextFieldDelegate
{
public:
    virtual ~TextDelegate2() = default;
    bool TextFieldKeyPressed(UITextField* /*textField*/, int32 replacementLocation, int32 replacementLength, WideString& replacementString) override
    {
        // Allow only numbers
        bool accept = true;
        for (size_t i = 0, n = replacementString.length();i < n && accept;++i)
        {
            accept = '0' <= replacementString[i] && replacementString[i] <= '9';
        }
        Logger::Debug("****** TextDelegate1::TextFieldKeyPressed: accepted=%d", accept);
        return accept;
    }
    void TextFieldOnTextChanged(UITextField* /*textField*/, const WideString& newText, const WideString& oldText) override
    {
        Logger::Debug("****** TextDelegate2::TextFieldOnTextChanged: new=%s, old=%s", WStringToString(newText).c_str(), WStringToString(oldText).c_str());
    }
};

class TextDelegateMulti : public UITextFieldDelegate
{
public:
    virtual ~TextDelegateMulti() = default;
};

MultilineTest::MultilineTest ()
    : BaseScreen("MultilineTest")
{
}

void MultilineTest::LoadResources()
{
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(14);

    textDelegate1 = new TextDelegate1;
    textDelegate2 = new TextDelegate2;

    textField1 = new UITextField(Rect(5, 10, 400, 60));
    textField1->SetFont(font);
    textField1->SetText(L"Hello World");
    textField1->SetDebugDraw(true);
    textField1->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    textField1->SetDelegate(textDelegate1);
    textField1->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);

    textField2 = new UITextField(Rect(5, 80, 400, 60));
    textField2->SetIsPassword(true);
    textField2->SetFont(font);
    textField2->SetText(L"Die my darling");
    textField2->SetDebugDraw(true);
    textField2->SetTextColor(Color(0.0, 0.0, 1.0, 1.0));
    textField2->SetKeyboardType(UITextField::eKeyboardType::KEYBOARD_TYPE_NUMBER_PAD);
    textField2->SetDelegate(textDelegate2);
    textField2->SetTextAlign(ALIGN_RIGHT | ALIGN_TOP);

    textFieldMulti = new UITextField(Rect(450, 10, 400, 120));
    textFieldMulti->SetFont(font);
    textFieldMulti->SetText(L"Multiline text field");
    textFieldMulti->SetDebugDraw(true);
    textFieldMulti->SetTextColor(Color(0.0, 0.0, 1.0, 1.0));
    textFieldMulti->SetMultiline(true);
    textFieldMulti->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);

    AddControl(textField1);
    AddControl(textField2);
    AddControl(textFieldMulti);

    
    const uint32 Y_OFFSET = 500;
    const uint32 CONTROL_LENGHT = 400;
    const uint32 CONTROL_HEIGTH = 70;
    
    BaseScreen::LoadResources();
    
    font->SetSize(25.f);
    ScopedPtr<FTFont> bigFont(FTFont::Create("~res:/Fonts/korinna.ttf"));
    bigFont->SetSize(50.f);
    
    UIButton * button = new UIButton(Rect(0,Y_OFFSET, CONTROL_LENGHT, CONTROL_HEIGTH));
    button->SetStateFont(0xFF, font);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetStateText(0xFF, L"Show/Hide");
    button->SetDebugDraw(true);
    button->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &MultilineTest::OnShowHideClick));
    AddControl(button);
    SafeRelease(button);
    
    UITextField * field = new UITextField(Rect(0, Y_OFFSET+CONTROL_HEIGTH+10, CONTROL_LENGHT, CONTROL_HEIGTH));
    field->SetFont(font);
    field->SetDebugDraw(true);
    field->SetText(L"Test text inside UITextField used for test");
    field->SetDelegate(this);
    AddControl(field);
    SafeRelease(field);
    
    field = new UITextField(Rect(0, Y_OFFSET+2*(CONTROL_HEIGTH+10), CONTROL_LENGHT, CONTROL_HEIGTH));
    field->SetFont(font);
    field->SetFocused();
    field->SetDebugDraw(true);
    field->SetText(L"Test text inside UITextField used for test");
    field->SetDelegate(this);
    
    AddControl(field);
    SafeRelease(field);
    
    topLayerControl = new UIControl(Rect(CONTROL_LENGHT/3, Y_OFFSET, CONTROL_LENGHT/3, 3*(Y_OFFSET+CONTROL_HEIGTH+10)));
    topLayerControl->GetBackground()->SetColor(Color(1.0f, 0.0f, 0.0f, 0.5f));
    topLayerControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    topLayerControl->GetBackground()->SetColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    topLayerControl->SetDebugDraw(true);
    AddControl(topLayerControl);
    
    BaseScreen::LoadResources();
}

void MultilineTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(textField1);
    SafeRelease(textField2);
    SafeRelease(textFieldMulti);

    SafeDelete(textDelegate1);
    SafeDelete(textDelegate2);
    SafeRelease(topLayerControl);
}

void MultilineTest::OnShowHideClick(BaseObject* sender, void * data, void * callerData)
{
    if (nullptr != topLayerControl)
    {
        static bool isVisible = true;
        topLayerControl->SetVisible(isVisible);
        isVisible = !isVisible;
    }
}

UIButton* MultilineTest::CreateUIButton(Font* font, const Rect& rect, const String& text,
                                        void (MultilineTest::*onClick)(BaseObject*, void*, void*))
{
    UIButton* button = new UIButton(rect);
    button->SetStateFont(0xFF, font);
    button->SetStateText(0xFF, StringToWString(text));
    button->SetStateFontColor(0xFF, Color::White);
    button->SetDebugDraw(true);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, onClick));
    AddControl(button);
    return button;
}
