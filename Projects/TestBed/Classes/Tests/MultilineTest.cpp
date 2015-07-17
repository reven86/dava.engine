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

MultilineTest::MultilineTest ()
    : BaseScreen("MultilineTest")
{
}

void MultilineTest::LoadResources()
{
    textField = new UITextField(Rect(100, 60, 200, 60));

#if defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_MACOS__)
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(14);
    textField->SetFont(font);
#endif

    textField->SetText(L"Hello World");
    textField->SetDebugDraw(true);
    textField->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));

    static UITextFieldDelegate delegate;

    textField->SetDelegate(&delegate);
    textField->SetMultiline(true);
    textField->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);

    //textField->SetSpellCheckingType(UITextField::SPELL_CHECKING_TYPE_NO);
    textField->SetMaxLength(-1);

    AddControl(textField);

    showKbd = CreateUIButton(font, Rect(80, 0, 50, 20), "Show", &MultilineTest::OnShow);
    hideKbd = CreateUIButton(font, Rect(80, 30, 50, 20), "Hide", &MultilineTest::OnHide);

    BaseScreen::LoadResources();
}

void MultilineTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(textField);
    SafeRelease(showKbd);
    SafeRelease(hideKbd);
}

void MultilineTest::OnShow(DAVA::BaseObject*, void*, void*)
{
    textField->OpenKeyboard();
}

void MultilineTest::OnHide(DAVA::BaseObject*, void*, void*)
{
    textField->CloseKeyboard();
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
