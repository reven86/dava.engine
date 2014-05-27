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

uint32_t UITextFieldImpl::sId = 0;
DAVA::Map<uint32_t, UITextFieldImpl*> UITextFieldImpl::controls;

UITextFieldImpl::UITextFieldImpl(UITextField* tf)
{
	textField = tf;
	id = sId++;
	rect = textField->GetRect();
	JniTextField jniTextField(id);
	jniTextField.Create(rect);

	controls[id] = this;
}

UITextFieldImpl::~UITextFieldImpl()
{
	controls.erase(id);

	JniTextField jniTextField(id);
	jniTextField.Destroy();
}

void UITextFieldImpl::OpenKeyboard()
{
	JniTextField jniTextField(id);
	jniTextField.OpenKeyboard();
}

void UITextFieldImpl::CloseKeyboard()
{
	JniTextField jniTextField(id);
	jniTextField.CloseKeyboard();
}

void UITextFieldImpl::GetText(WideString & string) const
{
	string = text;
}

void UITextFieldImpl::SetText(const WideString & string)
{
	if (text.compare(string) != 0)
	{
		text = string;
		JniTextField jniTextField(id);
		String utfText = WStringToString(text);
		jniTextField.SetText(utfText.c_str());
	}
}

void UITextFieldImpl::UpdateRect(const Rect & newRect, float32 timeElapsed)
{
	if (newRect != rect)
	{
		rect = newRect;
		JniTextField jniTextField(id);
		jniTextField.UpdateRect(rect);
	}
}
void UITextFieldImpl::GetTextColor(DAVA::Color &color) const
{

}

void UITextFieldImpl::SetTextColor(const DAVA::Color &color)
{
	JniTextField jniTextField(id);
	jniTextField.SetTextColor(color.r, color.g, color.b, color.a);
}
Font *UITextFieldImpl::GetFont(){ return NULL; }

void UITextFieldImpl::SetFont(Font * font){}

void UITextFieldImpl::SetFontSize(float size)
{
	JniTextField jniTextField(id);
	jniTextField.SetFontSize(size);
}

void UITextFieldImpl::SetTextAlign(DAVA::int32 newAlign)
{
	align = newAlign;
	JniTextField jniTextField(id);
	jniTextField.SetTextAlign(align);
}

int32 UITextFieldImpl::GetTextAlign() const
{
	return align;
}

void UITextFieldImpl::ShowField()
{
	JniTextField jniTextField(id);
	jniTextField.ShowField();
}

void UITextFieldImpl::HideField()
{
	JniTextField jniTextField(id);
	jniTextField.HideField();
}
/*
void UITextFieldImpl::SetVisible(bool isVisible)
{
	if (isVisible)
		ShowField();
	else
		HideField();
}
*/
void UITextFieldImpl::SetIsPassword(bool isPassword)
{
	JniTextField jniTextField(id);
	jniTextField.SetIsPassword(isPassword);
}

void UITextFieldImpl::SetInputEnabled(bool value)
{
	JniTextField jniTextField(id);
	jniTextField.SetInputEnabled(value);
}

// Keyboard traits.
void UITextFieldImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
	JniTextField jniTextField(id);
	jniTextField.SetAutoCapitalizationType(value);
}

void UITextFieldImpl::SetAutoCorrectionType(DAVA::int32 value)
{
	JniTextField jniTextField(id);
	jniTextField.SetAutoCorrectionType(value);
}

void UITextFieldImpl::SetSpellCheckingType(DAVA::int32 value)
{
	JniTextField jniTextField(id);
	jniTextField.SetSpellCheckingType(value);
}

void UITextFieldImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
	JniTextField jniTextField(id);
	jniTextField.SetKeyboardAppearanceType(value);
}

void UITextFieldImpl::SetKeyboardType(DAVA::int32 value)
{
	JniTextField jniTextField(id);
	jniTextField.SetKeyboardType(value);
}

void UITextFieldImpl::SetReturnKeyType(DAVA::int32 value)
{
	JniTextField jniTextField(id);
	jniTextField.SetReturnKeyType(value);
}

void UITextFieldImpl::SetEnableReturnKeyAutomatically(bool value)
{
	JniTextField jniTextField(id);
	jniTextField.SetEnableReturnKeyAutomatically(value);
}

uint32 UITextFieldImpl::GetCursorPos() const
{
	JniTextField jniTextField(id);
	return jniTextField.GetCursorPos();
}

void UITextFieldImpl::SetCursorPos(uint32 pos)
{
	JniTextField jniTextField(id);
	return jniTextField.SetCursorPos(pos);
}

bool UITextFieldImpl::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const WideString &text)
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

bool UITextFieldImpl::TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, const WideString &text)
{
	UITextFieldImpl* control = GetUITextFieldAndroid(id);
	if (!control)
		return false;

	return control->TextFieldKeyPressed(replacementLocation, replacementLength, text);
}

void UITextFieldImpl::TextFieldShouldReturn()
{
	UITextFieldDelegate* delegate = textField->GetDelegate();
	if (delegate)
		delegate->TextFieldShouldReturn(textField);
}

void UITextFieldImpl::TextFieldShouldReturn(uint32_t id)
{
	UITextFieldImpl* control = GetUITextFieldAndroid(id);
	if (!control)
		return;

	control->TextFieldShouldReturn();
}

UITextFieldImpl* UITextFieldImpl::GetUITextFieldAndroid(uint32_t id)
{
	DAVA::Map<uint32_t, UITextFieldImpl*>::iterator iter = controls.find(id);
	if (iter != controls.end())
		return iter->second;

	return NULL;
}
void UITextFieldImpl::Input(UIEvent *currentInput){}
void UITextFieldImpl::Draw(const UIGeometricData &geometricData){}
