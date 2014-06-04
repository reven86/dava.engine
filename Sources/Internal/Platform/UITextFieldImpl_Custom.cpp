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

#include "UITextFieldImpl_Custom.h"
#include "UI/UITextField.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlSystem.h"
#include "Input/KeyboardDevice.h"

namespace DAVA
{
static const float32 CURSOR_BLINK_PERIOD = 0.5f;

UITextFieldImpl_Custom::UITextFieldImpl_Custom(UITextField* tf)
    : UITextFieldImpl(tf)
    , needRedraw(false)
    , showCursor(true)
    , isPassword(false)
    , cursorTime(0.0f)
{
    staticText = new UIStaticText (textField->GetRect(true));
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
}

UITextFieldImpl_Custom::~UITextFieldImpl_Custom()
{
    SafeRelease(staticText);
}

WideString UITextFieldImpl_Custom::GetVisibleText() const
{
    if (!isPassword)
        return text;

    WideString passText = text;
    passText.replace(0, passText.length(), passText.length(), L'*');
    return passText;
}

const WideString & UITextFieldImpl_Custom::GetText() const
{
    return text;
}

void UITextFieldImpl_Custom::SetText(const WideString & string)
{
    text = string;
    needRedraw = true;
}

void UITextFieldImpl_Custom::UpdateRect(const Rect & newRect, float32 timeElapsed)
{
    if(newRect != staticText->GetRect(true))
    {
        staticText->SetRect(newRect, false);
        needRedraw = true;
    }
    
    if(textField == UIControlSystem::Instance()->GetFocusedControl())
    {
        cursorTime += timeElapsed;

        if (cursorTime >= CURSOR_BLINK_PERIOD)
        {
            cursorTime = 0;
            showCursor = !showCursor;
            needRedraw = true;
        }
    }
    
    if (!needRedraw)
        return;

    if(textField == UIControlSystem::Instance()->GetFocusedControl())
    {
        WideString txt = GetVisibleText();
        txt += showCursor ? L"|" : L" ";
        staticText->SetText(txt);
    }
    else
    {
        staticText->SetText(GetVisibleText());
    }
    needRedraw = false;
}

const Color &UITextFieldImpl_Custom::GetTextColor() const
{
    return staticText->GetTextColor();
}

void UITextFieldImpl_Custom::SetTextColor(const Color &color)
{
    staticText->SetTextColor(color);
}

Font * UITextFieldImpl_Custom::GetFont() const
{
    return staticText->GetFont();
}

void UITextFieldImpl_Custom::SetFont(Font * font)
{
    staticText->SetFont(font);
}

void UITextFieldImpl_Custom::SetFontSize(float32 size)
{
    ScopedPtr<Font> newFont(staticText->GetFont()->Clone());
    newFont->SetSize(size);
    SetFont(newFont);
}

void UITextFieldImpl_Custom::SetTextAlign(int32 align)
{
    staticText->SetTextAlign(align);
}

void UITextFieldImpl_Custom::SetIsPassword(bool isPasswordValue)
{
    needRedraw = true;
    isPassword = isPasswordValue;
}

uint32 UITextFieldImpl_Custom::GetCursorPos() const
{
    return 0;
}

void UITextFieldImpl_Custom::SetCursorPos(uint32 pos)
{

}

void UITextFieldImpl_Custom::Input(UIEvent *currentInput)
{
    UITextFieldDelegate * delegate = textField->GetDelegate();
    if (!delegate)
    {
        return;
    }

    if(textField != UIControlSystem::Instance()->GetFocusedControl())
        return;


    if (currentInput->phase == UIEvent::PHASE_KEYCHAR)
    {	
        /// macos
        if (currentInput->tid == DVKEY_LEFT||
            currentInput->tid == DVKEY_RIGHT||
            currentInput->tid == DVKEY_DOWN||
            currentInput->tid == DVKEY_UP)
        {
            ;//ignore arrows
        }
        else if (currentInput->tid == DVKEY_BACKSPACE)
        {
            //TODO: act the same way on iPhone
            WideString str = L"";
            if(delegate->TextFieldKeyPressed(textField, (int32)text.length() - 1, 1, str))
            {
                SetText(textField->GetAppliedChanges((int32)text.length() - 1,  1, str));
            }
        }
        else if (currentInput->tid == DVKEY_ENTER)
        {
            delegate->TextFieldShouldReturn(textField);
        }
        else if (currentInput->tid == DVKEY_ESCAPE)
        {
            delegate->TextFieldShouldCancel(textField);
        }
        else if(currentInput->keyChar != 0)
        {
            WideString str;
            str += currentInput->keyChar;
            if(delegate->TextFieldKeyPressed(textField, (int32)textField->GetText().length(), 0, str))
            {
                textField->SetText(textField->GetAppliedChanges((int32)textField->GetText().length(),  0, str));
            }
        }
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

void UITextFieldImpl_Custom::Draw()
{
    staticText->SystemDraw(UIControlSystem::Instance()->GetBaseGeometricData());
}

}