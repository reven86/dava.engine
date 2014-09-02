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



#ifndef __DAVAENGINE_UI_TEXT_FIELD_ANDROID_H__
#define __DAVAENGINE_UI_TEXT_FIELD_ANDROID_H__

#include "UI/UITextField.h"
#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "JniExtensions.h"

namespace DAVA
{

class JniTextField: public JniExtension
{
public:
	JniTextField(uint32_t id);

	void Create(Rect rect);
	void Destroy();
	void UpdateRect(const Rect & rect);
	void SetText(const char* text);
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

protected:
	virtual jclass GetJavaClass() const;
	virtual const char* GetJavaClassName() const;

public:
	static jclass gJavaClass;
	static const char* gJavaClassName;

private:
	uint32_t id;
};

class UITextFieldAndroid
{
public:
	UITextFieldAndroid(UITextField* textField);
	virtual ~UITextFieldAndroid();

	void OpenKeyboard();
	void CloseKeyboard();
	void GetText(WideString & string) const;
	void SetText(const WideString & string);
	void UpdateRect(const Rect & rect);

	void SetTextColor(const DAVA::Color &color);
	void SetFontSize(float size);

	void SetTextAlign(DAVA::int32 align);
	DAVA::int32 GetTextAlign();

	void ShowField();
	void HideField();
	void SetVisible(bool isVisible);

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

	uint32 GetCursorPos();
	void SetCursorPos(uint32 pos);

	bool TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const WideString &text);
	void TextFieldShouldReturn();
	static bool TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, const WideString &text);
	static void TextFieldShouldReturn(uint32_t id);

private:
	static UITextFieldAndroid* GetUITextFieldAndroid(uint32_t id);

private:
	UITextField* textField;
	static uint32_t sId;
	static DAVA::Map<uint32_t, UITextFieldAndroid*> controls;
	uint32_t id;
	Rect rect;
	WideString text;
	int32_t align;
};

};

#endif //__DAVAENGINE_ANDROID__


#endif //__DAVAENGINE_UI_TEXT_FIELD_ANDROID_H__
