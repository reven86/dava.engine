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

#include "UITextFieldImpl_Android.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/JniTextField.h"
#include "UI/UITextField.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{
static uint32_t sId = 0;
static Map<uint32_t, UITextFieldImpl_Android*> idToImpl;

UITextFieldImpl_Android* GetUITextFieldImpl(uint32_t id)
{
    Map<uint32_t, UITextFieldImpl_Android*>::iterator iter = idToImpl.find(id);
    if (iter != idToImpl.end())
        return iter->second;

    return NULL;
}

UITextFieldImpl_Android::UITextFieldImpl_Android(UITextField *tf)
    : UITextFieldImpl(tf)
{
    id = sId++;
    idToImpl[id] = this;
    JniTextField jniField(id);
    jniField.Create(textField->GetRect(true));
}

UITextFieldImpl_Android::~UITextFieldImpl_Android()
{
    JniTextField jniField(id);
    jniField.Destroy();
    idToImpl.erase(id);
}

void UITextFieldImpl_Android::UpdateRect(const Rect & controlRect, float32 timeElapsed)
{
    if (controlRect == rect)
        return;
    rect = controlRect;
    JniTextField jniField(id);
    jniField.UpdateRect(rect, timeElapsed);
}
const WideString &UITextFieldImpl_Android::GetText() const
{
    JniTextField jniField(id);
    jniField.GetText(text);
    return text;
}
void UITextFieldImpl_Android::SetText(const WideString &newText)
{
    JniTextField jniField(id);
    jniField.SetText(newText);
}

const Color & UITextFieldImpl_Android::GetTextColor() const
{
    return textColor;
}

void UITextFieldImpl_Android::SetTextColor(const Color &color)
{
    JniTextField jniField(id);
    jniField.SetTextColor(color);
}

void UITextFieldImpl_Android::SetFontSize(float32 size)
{
    JniTextField jniField(id);
    jniField.SetFontSize(size);
}

void UITextFieldImpl_Android::SetIsPassword(bool isPassword)
{
    JniTextField jniField(id);
    jniField.SetIsPassword(isPassword);
}

void UITextFieldImpl_Android::SetTextAlign(int32 align)
{
    JniTextField jniField(id);
    jniField.SetTextAlign(align);
}

void UITextFieldImpl_Android::SetInputEnabled(bool value)
{
    JniTextField jniField(id);
    jniField.SetInputEnabled(value);
}

void UITextFieldImpl_Android::SetAutoCapitalizationType(int32 value)
{
    JniTextField jniField(id);
    jniField.SetAutoCapitalizationType(value);
}

void UITextFieldImpl_Android::SetAutoCorrectionType(int32 value)
{
    JniTextField jniField(id);
    jniField.SetAutoCorrectionType(value);
}

void UITextFieldImpl_Android::SetSpellCheckingType(int32 value)
{
    JniTextField jniField(id);
    jniField.SetSpellCheckingType(value);
}

void UITextFieldImpl_Android::SetKeyboardAppearanceType(int32 value)
{
    JniTextField jniField(id);
    jniField.SetKeyboardAppearanceType(value);
}

void UITextFieldImpl_Android::SetKeyboardType(int32 value)
{
    JniTextField jniField(id);
    jniField.SetKeyboardType(value);
}

void UITextFieldImpl_Android::SetReturnKeyType(int32 value)
{
    JniTextField jniField(id);
    jniField.SetReturnKeyType(value);
}

void UITextFieldImpl_Android::SetEnableReturnKeyAutomatically(bool value)
{
    JniTextField jniField(id);
    jniField.SetEnableReturnKeyAutomatically(value);
}

void UITextFieldImpl_Android::RemoveNativeControl()
{
    //native control removed from view on destruction
}

void UITextFieldImpl_Android::AddNativeControl()
{
    //native control added to view on creation
}

void UITextFieldImpl_Android::SetVisible(bool value)
{
    JniTextField jniField(id);
    if(value)
        jniField.ShowField();
    else
        jniField.HideField();
}

void UITextFieldImpl_Android::OpenKeyboard()
{
    JniTextField jniField(id);
    jniField.OpenKeyboard();
}

void UITextFieldImpl_Android::CloseKeyboard()
{
    JniTextField jniField(id);
    jniField.CloseKeyboard();
}

uint32 UITextFieldImpl_Android::GetCursorPos() const
{
    JniTextField jniField(id);
    return jniField.GetCursorPos();
}

void UITextFieldImpl_Android::SetCursorPos(uint32 pos)
{
    JniTextField jniField(id);
    jniField.SetCursorPos(pos);
}

bool UITextFieldImpl_Android::TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, const WideString &text)
{
    UITextFieldImpl_Android* impl = GetUITextFieldImpl(id);
    if (!impl)
        return true;

    UITextField * control = impl->textField;

    if(!control || !control->GetDelegate())
        return true;

    return control->GetDelegate()->TextFieldKeyPressed(control, replacementLocation, replacementLength, text);
}

void UITextFieldImpl_Android::TextFieldShouldReturn(uint32_t id)
{
    UITextFieldImpl_Android* impl = GetUITextFieldImpl(id);
    if (!impl)
        return;

    UITextField * control = impl->textField;

    if(!control || !control->GetDelegate())
        return;

    control->GetDelegate()->TextFieldShouldReturn(control);
}
};
#endif //#if defined(__DAVAENGINE_ANDROID__)
