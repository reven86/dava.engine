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

#include "UI/UITextFieldImpl.h"
#include "UI/UITextField.h"
#include "UI/UIStaticText.h"
#include "Input/KeyboardDevice.h"

namespace DAVA
{

UITextFieldImpl::UITextFieldImpl(UITextField* tf)
    : needRedraw(false)
    , showCursor(true)
    , isPassword(false)
    , cursorTime(0.0f)
    , textFont(NULL)
{
    textField = tf;
    staticText = new UIStaticText(tf->GetRect(true));

    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
}

UITextFieldImpl::~UITextFieldImpl()
{
    SafeRelease(textFont);
    SafeRelease(staticText);
}

void UITextFieldImpl::OpenKeyboard()
{
}

void UITextFieldImpl::CloseKeyboard()
{
}

void UITextFieldImpl::GetText(WideString & string) const
{
	string = text;
}

void UITextFieldImpl::SetText(const WideString & string)
{
    text = string;
    needRedraw = true;
}

void UITextFieldImpl::UpdateRect(const Rect & newRect, float32 timeElapsed)
{
    if( newRect != staticText->GetRect(true) )
    {
        staticText->SetRect(newRect, false);
        needRedraw = true;
    }
    
    if(textField == UIControlSystem::Instance()->GetFocusedControl())
    {
        cursorTime += timeElapsed;

        if (cursorTime >= 0.5f)
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
        WideString txt = textField->GetVisibleText();
        txt += showCursor ? L"|" : L" ";
        staticText->SetText(txt);
	}
	else
    {
        staticText->SetText(textField->GetVisibleText());
    }
    needRedraw = false;
}

void UITextFieldImpl::GetTextColor( Color & color) const
{
    color = staticText->GetTextColor();
}

void UITextFieldImpl::SetTextColor(const DAVA::Color &color)
{
    staticText->SetTextColor(color);
}

Font *UITextFieldImpl::GetFont()
{
    return textFont;
}

void UITextFieldImpl::SetFont(Font * font)
{
    if( textFont == font )
        return;

    SafeRelease(textFont);
    textFont = SafeRetain(font);
    staticText->SetFont(textFont);
}

void UITextFieldImpl::SetFontSize(float32 size)
{
    textFont->SetSize(size);
    staticText->PrepareSprite();
}

void UITextFieldImpl::SetTextAlign(DAVA::int32 newAlign)
{
	align = newAlign;
    staticText->SetTextAlign(align);
}

DAVA::int32 UITextFieldImpl::GetTextAlign() const
{
	return align;
}

void UITextFieldImpl::ShowField()
{
}

void UITextFieldImpl::HideField()
{
}

void UITextFieldImpl::SetIsPassword(bool isPasswordValue)
{
    needRedraw = true;
    isPassword = isPasswordValue;
}

void UITextFieldImpl::SetInputEnabled(bool value)
{
}

// Keyboard traits.
void UITextFieldImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
}

void UITextFieldImpl::SetAutoCorrectionType(DAVA::int32 value)
{

}

void UITextFieldImpl::SetSpellCheckingType(DAVA::int32 value)
{

}

void UITextFieldImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{

}

void UITextFieldImpl::SetKeyboardType(DAVA::int32 value)
{

}

void UITextFieldImpl::SetReturnKeyType(DAVA::int32 value)
{

}

void UITextFieldImpl::SetEnableReturnKeyAutomatically(bool value)
{

}

uint32 UITextFieldImpl::GetCursorPos() const
{
    return 0;
}

void UITextFieldImpl::SetCursorPos(uint32 pos)
{
}

void UITextFieldImpl::Input(UIEvent *currentInput)
{
    if (NULL == textField->delegate)
    {
        return;
    }

    if(textField != UIControlSystem::Instance()->GetFocusedControl())
        return;


    if (currentInput->phase == UIEvent::PHASE_KEYCHAR)
    {	
        /// macos

        if (currentInput->tid == DVKEY_BACKSPACE)
        {
            //TODO: act the same way on iPhone
            WideString str = L"";
            if(textField->delegate->TextFieldKeyPressed(textField, (int32)textField->GetText().length() - 1, 1, str))
            {
                textField->SetText(textField->GetAppliedChanges((int32)textField->GetText().length() - 1,  1, str));
            }
        }
        else if (currentInput->tid == DVKEY_ENTER)
        {
            textField->delegate->TextFieldShouldReturn(textField);
        }
        else if (currentInput->tid == DVKEY_ESCAPE)
        {
            textField->delegate->TextFieldShouldCancel(textField);
        }
        else if(currentInput->keyChar != 0)
        {
            WideString str;
            str += currentInput->keyChar;
            if(textField->delegate->TextFieldKeyPressed(textField, (int32)textField->GetText().length(), 0, str))
            {
                textField->SetText(textField->GetAppliedChanges((int32)textField->GetText().length(),  0, str));
            }
        }
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
}

void UITextFieldImpl::Draw( const UIGeometricData & )
{
    staticText->SystemDraw( UIControlSystem::Instance()->GetBaseGeometricData() );
}

};
