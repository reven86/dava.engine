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
#include "Platform/TemplateAndroid/JniTextField.h"

using namespace DAVA;

UITextFieldImpl::UITextFieldImpl(UITextField* tf)
{
	textField = tf;
    JniTextField * ptr = new JniTextField(this);
    ptr->Create(textField->GetRect());
    nativeClassPtr = ptr;
}

UITextFieldImpl::~UITextFieldImpl()
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->Destroy();
    nativeClassPtr = NULL;
    SafeDelete(ptr);
}

void UITextFieldImpl::OpenKeyboard()
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->OpenKeyboard();
}

void UITextFieldImpl::CloseKeyboard()
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->CloseKeyboard();
}

void UITextFieldImpl::GetText(WideString & string) const
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    string = ptr->GetText();
}

void UITextFieldImpl::SetText(const WideString & string)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetText(string);
}

void UITextFieldImpl::UpdateRect(const Rect & newRect, float32 timeElapsed)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->UpdateRect(newRect);
}
void UITextFieldImpl::GetTextColor(Color &color) const
{

}

void UITextFieldImpl::SetTextColor(const Color &color)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetTextColor(color.r, color.g, color.b, color.a);
}
Font *UITextFieldImpl::GetFont(){ return NULL; }

void UITextFieldImpl::SetFont(Font * font){}

void UITextFieldImpl::SetFontSize(float size)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetFontSize(size);
}

void UITextFieldImpl::SetTextAlign(int32 newAlign)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetTextAlign(newAlign);
}

void UITextFieldImpl::ShowField()
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->ShowField();
}

void UITextFieldImpl::HideField()
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->HideField();
}

void UITextFieldImpl::SetIsPassword(bool isPassword)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetIsPassword(isPassword);
}

void UITextFieldImpl::SetInputEnabled(bool value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetInputEnabled(value);
}

// Keyboard traits.
void UITextFieldImpl::SetAutoCapitalizationType(int32 value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetAutoCapitalizationType(value);
}

void UITextFieldImpl::SetAutoCorrectionType(int32 value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetAutoCorrectionType(value);
}

void UITextFieldImpl::SetSpellCheckingType(int32 value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetSpellCheckingType(value);
}

void UITextFieldImpl::SetKeyboardAppearanceType(int32 value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetKeyboardAppearanceType(value);
}

void UITextFieldImpl::SetKeyboardType(int32 value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetKeyboardType(value);
}

void UITextFieldImpl::SetReturnKeyType(int32 value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetReturnKeyType(value);
}

void UITextFieldImpl::SetEnableReturnKeyAutomatically(bool value)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    ptr->SetEnableReturnKeyAutomatically(value);
}

uint32 UITextFieldImpl::GetCursorPos() const
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    return ptr->GetCursorPos();
}

void UITextFieldImpl::SetCursorPos(uint32 pos)
{
    JniTextField * ptr = static_cast<JniTextField *>(nativeClassPtr);
    return ptr->SetCursorPos(pos);
}

// bool UITextFieldImpl::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const WideString &text)
// {
// 	bool res = true;
// 	UITextFieldDelegate* delegate = textField->GetDelegate();
// 	if (delegate)
// 		res = delegate->TextFieldKeyPressed(textField, replacementLocation, replacementLength, text);
// 
// 	if (res)
// 	{
// 		WideString curText = textField->GetText();
// 		if (curText.length() >= replacementLocation)
// 		{
// 			curText.replace(replacementLocation, replacementLength, text);
// 			SetText( curText );
// 		}
// 	}
// 	return res;
// }
//
// void UITextFieldImpl::TextFieldShouldReturn()
// {
// 	UITextFieldDelegate* delegate = textField->GetDelegate();
// 	if (delegate)
// 		delegate->TextFieldShouldReturn(textField);
// }
//

void UITextFieldImpl::Input(UIEvent *currentInput){}
void UITextFieldImpl::Draw(const UIGeometricData &geometricData){}
