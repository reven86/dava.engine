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


#include "UITextFieldAndroid.h"
#include "Utils/UTF8Utils.h"

using namespace DAVA;

JniTextField::JniTextField(uint32_t id)
    : jniTextField("com/dava/framework/JNITextField")
{
    this->id = id;

    create = jniTextField.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("Create");
    destroy = jniTextField.GetStaticMethod<void, jint>("Destroy");
    updateRect = jniTextField.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("UpdateRect");
    setText = jniTextField.GetStaticMethod<void, jint, jstring>("SetText");
    setTextColor = jniTextField.GetStaticMethod<void, jint, jfloat, jfloat, jfloat, jfloat>("SetTextColor");
    setFontSize = jniTextField.GetStaticMethod<void, jint, jfloat>("SetFontSize");
    setIsPassword = jniTextField.GetStaticMethod<void, jint, jboolean>("SetIsPassword");
    setTextAlign = jniTextField.GetStaticMethod<void, jint, jint>("SetTextAlign");
    setTextUseRtlAlign = jniTextField.GetStaticMethod<void, jint, jboolean>("SetTextUseRtlAlign");
    setInputEnabled = jniTextField.GetStaticMethod<void, jint, jboolean>("SetInputEnabled");
    setAutoCapitalizationType = jniTextField.GetStaticMethod<void, jint, jint>("SetAutoCapitalizationType");
    setAutoCorrectionType = jniTextField.GetStaticMethod<void, jint, jint>("SetAutoCorrectionType");
    setSpellCheckingType = jniTextField.GetStaticMethod<void, jint, jint>("SetSpellCheckingType");
    setKeyboardAppearanceType = jniTextField.GetStaticMethod<void, jint, jint>("SetKeyboardAppearanceType");
    setKeyboardType = jniTextField.GetStaticMethod<void, jint, jint>("SetKeyboardType");
    setReturnKeyType = jniTextField.GetStaticMethod<void, jint, jint>("SetReturnKeyType");
    setEnableReturnKeyAutomatically = jniTextField.GetStaticMethod<void, jint, jboolean>("SetEnableReturnKeyAutomatically");
    setVisible = jniTextField.GetStaticMethod<void, jint, jboolean>("SetVisible");
    setRenderToTexture = jniTextField.GetStaticMethod<void, jint, jboolean>("SetRenderToTexture");
    isRenderToTexture = jniTextField.GetStaticMethod<jboolean, jint>("IsRenderToTexture");
    openKeyboard = jniTextField.GetStaticMethod<void, jint>("OpenKeyboard");
    closeKeyboard = jniTextField.GetStaticMethod<void, jint>("CloseKeyboard");
    getCursorPos = jniTextField.GetStaticMethod<jint, jint>("GetCursorPos");
    setCursorPos = jniTextField.GetStaticMethod<void, jint, jint>("SetCursorPos");
    setMaxLength = jniTextField.GetStaticMethod<void, jint, jint>("SetMaxLength");
}

void JniTextField::Create(Rect controlRect)
{
    Rect rect = JNI::V2P(controlRect);
    create(id, rect.x, rect.y, rect.dx,    rect.dy);
}

void JniTextField::Destroy()
{
    destroy(id);
}

void JniTextField::UpdateRect(const Rect & controlRect)
{
    Rect rect = JNI::V2P(controlRect);
    updateRect(id, rect.x, rect.y, rect.dx, rect.dy);
}

void JniTextField::SetText(const char* text)
{
    JNIEnv *env = JNI::GetEnv();
    jstring jStrDefaultText = env->NewStringUTF(text);
    setText(id, jStrDefaultText);
    env->DeleteLocalRef(jStrDefaultText);
}

void JniTextField::SetTextColor(float r, float g, float b, float a)
{
    setTextColor(id, r, g, b, a);
}

void JniTextField::SetFontSize(float size)
{
    setFontSize(id, VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalY(size));
}

void JniTextField::SetIsPassword(bool isPassword)
{
    setIsPassword(id, isPassword);
}

void JniTextField::SetTextAlign(int32_t align)
{
    setTextAlign(id, align);
}

void JniTextField::SetTextUseRtlAlign(bool useRtlAlign)
{
    setTextUseRtlAlign(id, useRtlAlign);
}

void JniTextField::SetInputEnabled(bool value)
{
    setInputEnabled(id, value);
}

void JniTextField::SetAutoCapitalizationType(int32_t value)
{
    setAutoCapitalizationType(id, value);
}

void JniTextField::SetAutoCorrectionType(int32_t value)
{
    setAutoCorrectionType(id, value);
}

void JniTextField::SetSpellCheckingType(int32_t value)
{
    setSpellCheckingType(id, value);
}

void JniTextField::SetKeyboardAppearanceType(int32_t value)
{
    setKeyboardAppearanceType(id, value);
}

void JniTextField::SetKeyboardType(int32_t value)
{
    setKeyboardType(id, value);
}

void JniTextField::SetReturnKeyType(int32_t value)
{
    setReturnKeyType(id, value);
}

void JniTextField::SetEnableReturnKeyAutomatically(bool value)
{
    setEnableReturnKeyAutomatically(id, value);
}

void JniTextField::SetVisible(bool isVisible)
{
    setVisible(id, isVisible);
}

void JniTextField::SetRenderToTexture(bool value)
{
    setRenderToTexture(id, value);
}

bool JniTextField::IsRenderToTexture() const
{
    return JNI_TRUE == isRenderToTexture(id);
}

void JniTextField::OpenKeyboard()
{
    openKeyboard(id);
}

void JniTextField::CloseKeyboard()
{
    closeKeyboard(id);
}

uint32 JniTextField::GetCursorPos()
{
    return getCursorPos(id);
}

void JniTextField::SetCursorPos(uint32 pos)
{
    setCursorPos(id, pos);
}

void JniTextField::SetMaxLength(int32_t value)
{
    setMaxLength(id, value);
}

uint32_t UITextFieldAndroid::sId = 0;
DAVA::Map<uint32_t, UITextFieldAndroid*> UITextFieldAndroid::controls;

UITextFieldAndroid::UITextFieldAndroid(UITextField* textField)
{
    this->textField = textField;
    id = sId++;
    rect = textField->GetRect();
    JniTextField jniTextField(id);
    jniTextField.Create(rect);

    controls[id] = this;
}

UITextFieldAndroid::~UITextFieldAndroid()
{
    controls.erase(id);

    JniTextField jniTextField(id);
    jniTextField.Destroy();
}

void UITextFieldAndroid::OpenKeyboard()
{
    JniTextField jniTextField(id);
    jniTextField.OpenKeyboard();
}

void UITextFieldAndroid::CloseKeyboard()
{
    JniTextField jniTextField(id);
    jniTextField.CloseKeyboard();
}

void UITextFieldAndroid::GetText(WideString & string) const
{
    string = text;
}

void UITextFieldAndroid::SetText(const WideString & string)
{
    if (text.compare(string) != 0)
    {
        text = TruncateText(string, textField->GetMaxLength());

        JniTextField jniTextField(id);
        String utfText = UTF8Utils::EncodeToUTF8(text);
        jniTextField.SetText(utfText.c_str());
    }
}

void UITextFieldAndroid::UpdateRect(const Rect & rect)
{
    if (rect != this->rect)
    {
        this->rect = rect;
        JniTextField jniTextField(id);
        jniTextField.UpdateRect(rect);
    }
}

void UITextFieldAndroid::SetTextColor(const DAVA::Color &color)
{
    JniTextField jniTextField(id);
    jniTextField.SetTextColor(color.r, color.g, color.b, color.a);
}

void UITextFieldAndroid::SetFontSize(float size)
{
    JniTextField jniTextField(id);
    jniTextField.SetFontSize(size);
}

void UITextFieldAndroid::SetTextAlign(DAVA::int32 align)
{
    this->align = align;
    JniTextField jniTextField(id);
    jniTextField.SetTextAlign(align);
}

DAVA::int32 UITextFieldAndroid::GetTextAlign()
{
    return align;
}

void UITextFieldAndroid::SetTextUseRtlAlign(bool useRtlAlign)
{
    this->useRtlAlign = useRtlAlign;
    JniTextField jniTextField(id);
    jniTextField.SetTextUseRtlAlign(useRtlAlign);
}

bool UITextFieldAndroid::GetTextUseRtlAlign() const
{
    return useRtlAlign;
}

void UITextFieldAndroid::SetVisible(bool isVisible)
{
    JniTextField jniTextField(id);
    jniTextField.SetVisible(isVisible);
}

void UITextFieldAndroid::SetIsPassword(bool isPassword)
{
    JniTextField jniTextField(id);
    jniTextField.SetIsPassword(isPassword);
}

void UITextFieldAndroid::SetInputEnabled(bool value)
{
    JniTextField jniTextField(id);
    jniTextField.SetInputEnabled(value);
}

void UITextFieldAndroid::SetRenderToTexture(bool value)
{
    JniTextField  jniTextField(id);
    jniTextField.SetRenderToTexture(value);
}

bool UITextFieldAndroid::IsRenderToTexture() const
{
    JniTextField jniTextField(id);
    return jniTextField.IsRenderToTexture();
}

// Keyboard traits.
void UITextFieldAndroid::SetAutoCapitalizationType(DAVA::int32 value)
{
    JniTextField jniTextField(id);
    jniTextField.SetAutoCapitalizationType(value);
}

void UITextFieldAndroid::SetAutoCorrectionType(DAVA::int32 value)
{
    JniTextField jniTextField(id);
    jniTextField.SetAutoCorrectionType(value);
}

void UITextFieldAndroid::SetSpellCheckingType(DAVA::int32 value)
{
    JniTextField jniTextField(id);
    jniTextField.SetSpellCheckingType(value);
}

void UITextFieldAndroid::SetKeyboardAppearanceType(DAVA::int32 value)
{
    JniTextField jniTextField(id);
    jniTextField.SetKeyboardAppearanceType(value);
}

void UITextFieldAndroid::SetKeyboardType(DAVA::int32 value)
{
    JniTextField jniTextField(id);
    jniTextField.SetKeyboardType(value);
}

void UITextFieldAndroid::SetReturnKeyType(DAVA::int32 value)
{
    JniTextField jniTextField(id);
    jniTextField.SetReturnKeyType(value);
}

void UITextFieldAndroid::SetEnableReturnKeyAutomatically(bool value)
{
    JniTextField jniTextField(id);
    jniTextField.SetEnableReturnKeyAutomatically(value);
}

uint32 UITextFieldAndroid::GetCursorPos()
{
    JniTextField jniTextField(id);
    return jniTextField.GetCursorPos();
}

void UITextFieldAndroid::SetCursorPos(uint32 pos)
{
    JniTextField jniTextField(id);
    jniTextField.SetCursorPos(pos);
}

void UITextFieldAndroid::SetMaxLength(DAVA::int32 value)
{
    JniTextField jniTextField(id);

    WideString truncated = TruncateText(text, value);
    if (truncated != text)
    {
        SetText(truncated);
    }

    return jniTextField.SetMaxLength(value);
}

WideString UITextFieldAndroid::TruncateText(const WideString& text, int32 maxLength)
{
    WideString str = text;

    if (maxLength >= 0 && maxLength < str.length())
    {
        str.resize(maxLength);
    }

    return str;
}

bool UITextFieldAndroid::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, WideString &text)
{
    bool res = true;
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        res = delegate->TextFieldKeyPressed(textField, replacementLocation, replacementLength, text);

    if (res)
    {
        WideString curText = textField->GetText();
        if (curText.length() >= replacementLocation)
        {
            curText.replace(replacementLocation, replacementLength, text);
            this->text = curText;
        }
    }
    return res;
}

bool UITextFieldAndroid::TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, WideString &text)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if (!control)
        return false;

    return control->TextFieldKeyPressed(replacementLocation, replacementLength, text);
}

void UITextFieldAndroid::TextFieldOnTextChanged(const WideString& newText, const WideString& oldText)
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
    {
        delegate->TextFieldOnTextChanged(textField, newText, oldText);
    }
}

void UITextFieldAndroid::TextFieldOnTextChanged(uint32_t id, const WideString& newText, const WideString& oldText)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if (!control)
    {
        return;
    }
    control->TextFieldOnTextChanged(newText, oldText);
}

void UITextFieldAndroid::TextFieldShouldReturn()
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->TextFieldShouldReturn(textField);
}

void UITextFieldAndroid::TextFieldShouldReturn(uint32_t id)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if (!control)
        return;

    control->TextFieldShouldReturn();
}

UITextFieldAndroid* UITextFieldAndroid::GetUITextFieldAndroid(uint32_t id)
{
    DAVA::Map<uint32_t, UITextFieldAndroid*>::iterator iter = controls.find(id);
    if (iter != controls.end())
        return iter->second;

    return NULL;
}

void UITextFieldAndroid::TextFieldKeyboardShown(const Rect& rect)
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->OnKeyboardShown(rect);
}

void UITextFieldAndroid::TextFieldKeyboardShown(uint32_t id, const Rect& rect)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if (!control)
        return;
    control->TextFieldKeyboardShown(rect);
}

void UITextFieldAndroid::TextFieldKeyboardHidden()
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->OnKeyboardHidden();
}

void UITextFieldAndroid::TextFieldKeyboardHidden(uint32_t id)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if (!control)
        return;
    control->TextFieldKeyboardHidden();
}

void UITextFieldAndroid::TextFieldFocusChanged(bool hasFocus)
{
    if(textField)
    {
        if(hasFocus)
        {
            if (DAVA::UIControlSystem::Instance()->GetFocusedControl() != textField)
            {
                DAVA::UIControlSystem::Instance()->SetFocusedControl(textField, false);
            }
        }
        else
        {
            if (DAVA::UIControlSystem::Instance()->GetFocusedControl() == textField)
            {
                DAVA::UIControlSystem::Instance()->SetFocusedControl(NULL, false);
            }
        }
    }
}

void UITextFieldAndroid::TextFieldFocusChanged(uint32_t id, bool hasFocus)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if(nullptr != control)
    {
        control->TextFieldFocusChanged(hasFocus);
    }
}

void UITextFieldAndroid::TextFieldUpdateTexture(uint32_t id, int32* rawPixels,
        int width, int height)
{
    UITextFieldAndroid* control = GetUITextFieldAndroid(id);
    if (nullptr != control)
    {
        UITextField& textField = *control->textField;

        if (rawPixels)
        {
            Texture* tex = Texture::CreateFromData(FORMAT_RGBA8888,
                    reinterpret_cast<uint8*>(rawPixels), width, height, false);
            SCOPE_EXIT{SafeRelease(tex);};

            Rect rect = textField.GetRect();
            Sprite* spr = Sprite::CreateFromTexture(tex, 0, 0, rect.dx,
                    rect.dy);
            SCOPE_EXIT{SafeRelease(spr);};

            textField.GetBackground()->SetSprite(spr, 0);
        }
        else
        {
            // reset sprite to prevent render old sprite under android view
            textField.SetSprite(nullptr, 0);
        }
    }
}
