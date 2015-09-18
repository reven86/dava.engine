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
#include "Render/Image/Image.h"
#include "Render/Image/ImageConvert.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/UIControlSystem.h"

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
    setMultiline = jniTextField.GetStaticMethod<void, jint, jboolean>("SetMultiline");
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

void JniTextField::SetMultiline(bool value)
{
    jboolean isMulti = static_cast<jboolean>(value);
    setMultiline(id, isMulti);
}

uint32_t TextFieldPlatformImpl::sId = 0;
DAVA::Map<uint32_t, TextFieldPlatformImpl*> TextFieldPlatformImpl::controls;

TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* textField)
{
    this->textField = textField;
    id = sId++;
    rect = textField->GetRect();
    jniTextField = std::make_shared<JniTextField>(id);
    jniTextField->Create(rect);

    controls[id] = this;
}

TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    controls.erase(id);
    jniTextField->Destroy();
}

void TextFieldPlatformImpl::OpenKeyboard()
{
    jniTextField->OpenKeyboard();
}

void TextFieldPlatformImpl::CloseKeyboard()
{
    jniTextField->CloseKeyboard();
}

void TextFieldPlatformImpl::GetText(WideString & string) const
{
    string = text;
}

void TextFieldPlatformImpl::SetText(const WideString & string)
{
    if (text.compare(string) != 0)
    {
        text = TruncateText(string, textField->GetMaxLength());

        String utfText = UTF8Utils::EncodeToUTF8(text);
        jniTextField->SetText(utfText.c_str());
    }
}

void TextFieldPlatformImpl::UpdateRect(const Rect & rect)
{
    if (rect != this->rect)
    {
        this->rect = rect;
        jniTextField->UpdateRect(rect);
    }
}

void TextFieldPlatformImpl::SetTextColor(const DAVA::Color &color)
{
    jniTextField->SetTextColor(color.r, color.g, color.b, color.a);
}

void TextFieldPlatformImpl::SetFontSize(float size)
{
    jniTextField->SetFontSize(size);
}

void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
{
    this->align = align;
    jniTextField->SetTextAlign(align);
}

DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return align;
}

void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
    this->useRtlAlign = useRtlAlign;
    jniTextField->SetTextUseRtlAlign(useRtlAlign);
}

bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return useRtlAlign;
}

void TextFieldPlatformImpl::SetVisible(bool isVisible)
{
    jniTextField->SetVisible(isVisible);
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
    jniTextField->SetIsPassword(isPassword);
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
    jniTextField->SetInputEnabled(value);
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
    jniTextField->SetRenderToTexture(value);
}

bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return jniTextField->IsRenderToTexture();
}

// Keyboard traits.
void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
    jniTextField->SetAutoCapitalizationType(value);
}

void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
{
    jniTextField->SetAutoCorrectionType(value);
}

void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
{
    jniTextField->SetSpellCheckingType(value);
}

void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
    jniTextField->SetKeyboardAppearanceType(value);
}

void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
{
    jniTextField->SetKeyboardType(value);
}

void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
{
    jniTextField->SetReturnKeyType(value);
}

void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
    jniTextField->SetEnableReturnKeyAutomatically(value);
}

uint32 TextFieldPlatformImpl::GetCursorPos()
{
    return jniTextField->GetCursorPos();
}

void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
    jniTextField->SetCursorPos(pos);
}

void TextFieldPlatformImpl::SetMaxLength(DAVA::int32 value)
{
    WideString truncated = TruncateText(text, value);
    if (truncated != text)
    {
        SetText(truncated);
    }

    return jniTextField->SetMaxLength(value);
}

void TextFieldPlatformImpl::SetMultiline(bool value)
{
	jniTextField->SetMultiline(value);
}

WideString TextFieldPlatformImpl::TruncateText(const WideString& text, int32 maxLength)
{
    WideString str = text;

    if (maxLength >= 0 && maxLength < str.length())
    {
        str.resize(maxLength);
    }

    return str;
}

bool TextFieldPlatformImpl::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, WideString &text)
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

bool TextFieldPlatformImpl::TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, WideString &text)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return false;

    return control->TextFieldKeyPressed(replacementLocation, replacementLength, text);
}

void TextFieldPlatformImpl::TextFieldOnTextChanged(const WideString& newText, const WideString& oldText)
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
    {
        delegate->TextFieldOnTextChanged(textField, newText, oldText);
    }
}

void TextFieldPlatformImpl::TextFieldOnTextChanged(uint32_t id, const WideString& newText, const WideString& oldText)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
    {
        return;
    }
    control->TextFieldOnTextChanged(newText, oldText);
}

void TextFieldPlatformImpl::TextFieldShouldReturn()
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->TextFieldShouldReturn(textField);
}

void TextFieldPlatformImpl::TextFieldShouldReturn(uint32_t id)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return;

    control->TextFieldShouldReturn();
}

TextFieldPlatformImpl* TextFieldPlatformImpl::GetUITextFieldAndroid(uint32_t id)
{
    DAVA::Map<uint32_t, TextFieldPlatformImpl*>::iterator iter = controls.find(id);
    if (iter != controls.end())
        return iter->second;

    return NULL;
}

void TextFieldPlatformImpl::TextFieldKeyboardShown(const Rect& rect)
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->OnKeyboardShown(rect);
}

void TextFieldPlatformImpl::TextFieldKeyboardShown(uint32_t id, const Rect& rect)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return;
    control->TextFieldKeyboardShown(rect);
}

void TextFieldPlatformImpl::TextFieldKeyboardHidden()
{
    UITextFieldDelegate* delegate = textField->GetDelegate();
    if (delegate)
        delegate->OnKeyboardHidden();
}

void TextFieldPlatformImpl::TextFieldKeyboardHidden(uint32_t id)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (!control)
        return;
    control->TextFieldKeyboardHidden();
}

void TextFieldPlatformImpl::TextFieldFocusChanged(bool hasFocus)
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

void TextFieldPlatformImpl::TextFieldFocusChanged(uint32_t id, bool hasFocus)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if(nullptr != control)
    {
        control->TextFieldFocusChanged(hasFocus);
    }
}

void TextFieldPlatformImpl::TextFieldUpdateTexture(uint32_t id, int32* rawPixels,
        int width, int height)
{
    TextFieldPlatformImpl* control = GetUITextFieldAndroid(id);
    if (nullptr != control)
    {
        UITextField& textField = *control->textField;

        if (nullptr != rawPixels)
        {
            // convert on the same memory
            uint32 pitch = width * 4;
            uint8* imageData = reinterpret_cast<uint8*>(rawPixels);
            ImageConvert::ConvertImageDirect(FORMAT_BGRA8888,
                    FORMAT_RGBA8888, imageData, width, height, pitch, imageData,
                    width, height, pitch);

            Texture* tex = Texture::CreateFromData(FORMAT_RGBA8888, imageData, width, height, false);
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
