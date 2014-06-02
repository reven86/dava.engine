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

#ifndef __DAVAENGINE_JNI_TEXT_FIELD_H__
#define __DAVAENGINE_JNI_TEXT_FIELD_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "JniExtensions.h"

namespace DAVA
{
class UITextFieldImpl;
class JniTextField: public JniExtension
{
public:
    JniTextField(UITextFieldImpl * impl);
    ~JniTextField();

    void Create(const Rect &rect);
    void Destroy();
    void UpdateRect(const Rect & rect);
    void GetText(WideString &text);
    void SetText(const WideString &text);
    void SetTextColor(float r, float g, float b, float a);
    void SetFontSize(float size);
    void SetIsPassword(bool isPassword);
    void SetTextAlign(int32_t align);
    void SetInputEnabled(bool value);
    void SetAutoCapitalizationType(int32_t value);
    void SetAutoCorrectionType(int32_t value);
    void SetSpellCheckingType(int32_t value);
    void SetKeyboardAppearanceType(int32_t value);
    void SetKeyboardType(int32_t value);
    void SetReturnKeyType(int32_t value);
    void SetEnableReturnKeyAutomatically(bool value);
    void ShowField();
    void HideField();
    void OpenKeyboard();
    void CloseKeyboard();
    uint32 GetCursorPos();
    void SetCursorPos(uint32 pos);

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
    WideString text;
    UITextFieldImpl *textFieldImpl;
};

};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif// __DAVAENGINE_JNI_TEXT_FIELD_H__
