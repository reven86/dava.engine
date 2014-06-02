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

#ifndef __DAVAENGINE_UI_TEXT_FIELD_IMPL_ANDROID_H__
#define __DAVAENGINE_UI_TEXT_FIELD_IMPL_ANDROID_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "UI/UITextFieldImpl.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "JniExtensions.h"

namespace DAVA
{
class UITextFieldImpl;
class UITextFieldImpl_Android: public UITextFieldImpl
                             , public JniExtension
{
public:
    UITextFieldImpl_Android(UITextField * textfield);
    ~UITextFieldImpl_Android();

    void Create(const Rect &rect);
    void Destroy();
    virtual void UpdateRect(const Rect & rect, float32 timeElapsed) override;
    virtual void GetText(WideString &text) const override;
    virtual void SetText(const WideString &text) override;
    virtual void SetTextColor(const Color &color) override;
    virtual void SetFontSize(float32 size) override;
    virtual void SetIsPassword(bool isPassword) override;
    virtual void SetTextAlign(int32 align) override;
    virtual void SetInputEnabled(bool value) override;
    virtual void SetAutoCapitalizationType(int32 value) override;
    virtual void SetAutoCorrectionType(int32 value) override;
    virtual void SetSpellCheckingType(int32 value) override;
    virtual void SetKeyboardAppearanceType(int32 value) override;
    virtual void SetKeyboardType(int32 value) override;
    virtual void SetReturnKeyType(int32 value) override;
    virtual void SetEnableReturnKeyAutomatically(bool value) override;
    virtual void ShowField() override;
    virtual void HideField() override;
    virtual void OpenKeyboard() override;
    virtual void CloseKeyboard() override;
    virtual uint32 GetCursorPos() const override;
    virtual void SetCursorPos(uint32 pos) override;

    static bool TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, const WideString &text);
    static void TextFieldShouldReturn(uint32_t id);

protected:
    virtual jclass GetJavaClass() const;
    virtual const char* GetJavaClassName() const;

public:
    static jclass gJavaClass;
    static const char* gJavaClassName;

private:
    uint32_t id;
    Rect rect;
};

};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif// __DAVAENGINE_UI_TEXT_FIELD_IMPL_ANDROID_H__
