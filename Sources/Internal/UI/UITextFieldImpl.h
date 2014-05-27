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
#ifndef __DAVAENGINE_UI_TEXT_FIELD_IMPL_H__
#define __DAVAENGINE_UI_TEXT_FIELD_IMPL_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA 
{
class UITextField;
class UIEvent;
class Font;
class UIGeometricData;
class UITextFieldImpl
{
public:
	UITextFieldImpl(UITextField * textField);
	virtual ~UITextFieldImpl();
	
	void OpenKeyboard();
	void CloseKeyboard();
	void GetText(WideString & string) const;
	void SetText(const WideString & string);
	void UpdateRect(const Rect & rect, float32 timeElapsed);

    void GetTextColor(DAVA::Color &color) const;
	void SetTextColor(const DAVA::Color &color);
    Font *GetFont();
    void SetFont(Font * font);
	void SetFontSize(float32 size);
    
    void SetTextAlign(DAVA::int32 align);
    DAVA::int32 GetTextAlign() const;

	void ShowField();
	void HideField();
	
	void SetIsPassword(bool isPassword);

	void SetInputEnabled(bool value);
    
	// Keyboard traits.
	void SetAutoCapitalizationType(DAVA::int32 value);
	void SetAutoCorrectionType(DAVA::int32 value);
	void SetSpellCheckingType(DAVA::int32 value);
	void SetKeyboardAppearanceType(DAVA::int32 value);
	void SetKeyboardType(DAVA::int32 value);
	void SetReturnKeyType(DAVA::int32 value);
	void SetEnableReturnKeyAutomatically(bool value);

    // Cursor pos.
    uint32 GetCursorPos() const;
    void SetCursorPos(uint32 pos);

    void Input(UIEvent *currentInput);
    void Draw(const UIGeometricData &geometricData);
#if defined(__DAVAENGINE_ANDROID__)
public:
    bool TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const WideString &text);
    void TextFieldShouldReturn();
    static bool TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, const WideString &text);
    static void TextFieldShouldReturn(uint32_t id);

private:
    static UITextFieldImpl* GetUITextFieldAndroid(uint32_t id);
    static uint32_t sId;
    static DAVA::Map<uint32_t, UITextFieldImpl*> controls;
    uint32_t id;
    Rect rect;
    WideString text;
    int32 align;
#elif defined(__DAVAENGINE_IPHONE__)

#else
    class UIStaticText * staticText;
    class Font * textFont;
    WideString text;

    bool needRedraw:1;
    bool showCursor:1;
    bool isPassword:1;
    float32 cursorTime;
    int32 align;
#endif

private:
	void * objcClassPtr;
    UITextField * textField;
};
};

#endif // __DAVAENGINE_UI_TEXT_FIELD_IMPL_H__