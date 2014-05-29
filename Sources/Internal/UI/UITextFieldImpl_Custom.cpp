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
#if !defined(__DAVAENGINE_ANDROID__) && !defined(__DAVAENGINE_IPHONE__)

#include "Platform/CustomTextField.h"

namespace DAVA
{

UITextFieldImpl::UITextFieldImpl(UITextField* tf)
{
    textField = tf;
    CustomTextField * ptr = new CustomTextField(this);
    nativeClassPtr = ptr;
}

UITextFieldImpl::~UITextFieldImpl()
{
    CustomTextField * customTF = static_cast<CustomTextField *>(nativeClassPtr);
    nativeClassPtr = NULL;
    SafeDelete(customTF);
}

void UITextFieldImpl::OpenKeyboard()
{
}

void UITextFieldImpl::CloseKeyboard()
{
}

void UITextFieldImpl::GetText(WideString & string) const
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->GetText(string);
}

void UITextFieldImpl::SetText(const WideString & string)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->SetText(string);
}

void UITextFieldImpl::UpdateRect(const Rect & newRect, float32 timeElapsed)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->UpdateRect( newRect, timeElapsed );
}

void UITextFieldImpl::GetTextColor( Color & color) const
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->GetTextColor( color );
}

void UITextFieldImpl::SetTextColor(const Color &color)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
}

Font *UITextFieldImpl::GetFont()
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    return ptr->GetFont();
}

void UITextFieldImpl::SetFont(Font * font)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->SetFont(font);
}

void UITextFieldImpl::SetFontSize(float32 size)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->SetFontSize(size);
}

void UITextFieldImpl::SetTextAlign(int32 align)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->SetTextAlign(align);
}

void UITextFieldImpl::ShowField()
{
}

void UITextFieldImpl::HideField()
{
}

void UITextFieldImpl::SetIsPassword(bool isPassword)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->SetIsPassword(isPassword);
}

void UITextFieldImpl::SetInputEnabled(bool value)
{
}

// Keyboard traits.
void UITextFieldImpl::SetAutoCapitalizationType(int32 value)
{
}

void UITextFieldImpl::SetAutoCorrectionType(int32 value)
{

}

void UITextFieldImpl::SetSpellCheckingType(int32 value)
{

}

void UITextFieldImpl::SetKeyboardAppearanceType(int32 value)
{

}

void UITextFieldImpl::SetKeyboardType(int32 value)
{

}

void UITextFieldImpl::SetReturnKeyType(int32 value)
{

}

void UITextFieldImpl::SetEnableReturnKeyAutomatically(bool value)
{

}

uint32 UITextFieldImpl::GetCursorPos() const
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    return ptr->GetCursorPos();
}

void UITextFieldImpl::SetCursorPos(uint32 pos)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->SetCursorPos(pos);
}

void UITextFieldImpl::Input(UIEvent *currentInput)
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->Input(currentInput);
}

void UITextFieldImpl::Draw( const UIGeometricData & )
{
    CustomTextField * ptr = static_cast<CustomTextField *>(nativeClassPtr);
    ptr->Draw();
}

};
#endif //#if !defined(__DAVAENGINE_ANDROID__) && !defined(__DAVAENGINE_IPHONE__)
