/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "UITextFieldAndroid.h"

using namespace DAVA;

jclass JniTextField::gJavaClass = NULL;
const char* JniTextField::gJavaClassName = NULL;

UITextField* JniTextField::activeTextField = NULL;

jclass JniTextField::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniTextField::GetJavaClassName() const
{
	return gJavaClassName;
}

void JniTextField::ShowField(UITextField* textField, const Rect& controlRect, const char* defaultText)
{
	Rect rect = V2P(controlRect);

	if (activeTextField)
		HideField();

	jmethodID mid = GetMethodID("ShowField", "(FFFFLjava/lang/String;ZIIIIII)V");
	if (mid)
	{
		jstring jStrDefaultText = GetEnvironment()->NewStringUTF(defaultText);
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				rect.x,
				rect.y,
				rect.dx,
				rect.dy,
				jStrDefaultText,
				textField->IsPassword(),
				textField->GetAutoCapitalizationType(),
				textField->GetAutoCorrectionType(),
				textField->GetSpellCheckingType(),
				textField->GetKeyboardAppearanceType(),
				textField->GetKeyboardType(),
				textField->GetReturnKeyType());
		GetEnvironment()->DeleteLocalRef(jStrDefaultText);
		activeTextField = textField;
		SafeRetain(activeTextField);
	}
}

void JniTextField::HideField()
{
	jmethodID mid = GetMethodID("HideField", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid);
	}
}

void JniTextField::FieldHiddenWithText(const char* text)
{
	if (!activeTextField)
		return;

	UITextField* _activeTextField = activeTextField;
	activeTextField = NULL;

	WideString strText = StringToWString(text);
	_activeTextField->SetText(strText);

	UIControl* curentActiveControl = UIControlSystem::Instance()->GetFocusedControl();
	if (curentActiveControl == _activeTextField)
	{
		UIControlSystem::Instance()->SetFocusedControl(_activeTextField->GetParent(), true);
	}
	SafeRelease(_activeTextField);
}

void JniTextField::TextFieldShouldReturn()
{
	DVASSERT(activeTextField);
	UITextFieldDelegate* delegate = activeTextField->GetDelegate();
	if (delegate)
		delegate->TextFieldShouldReturn(activeTextField);
}

bool JniTextField::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const char* text)
{
	if (!activeTextField)
		return false;

	bool res = true;
	WideString strText = StringToWString(text);
	UITextFieldDelegate* delegate = activeTextField->GetDelegate();
	if (delegate)
		res = delegate->TextFieldKeyPressed(activeTextField, replacementLocation, replacementLength, strText);

	if (res)
	{
		WideString curText = activeTextField->GetText();
		curText.replace(replacementLocation, replacementLength, strText);
		activeTextField->SetText(curText);
	}
	return res;
}

UITextFieldAndroid::UITextFieldAndroid(UITextField* textField)
{
	this->textField = textField;
}

UITextFieldAndroid::~UITextFieldAndroid()
{

}

void UITextFieldAndroid::ShowField()
{
	String text = WStringToString(textField->GetText());
	JniTextField jniTextField;
	jniTextField.ShowField(textField, textField->GetRect(), text.c_str());
}

void UITextFieldAndroid::HideField()
{
	JniTextField jniTextField;
	jniTextField.HideField();
}


//TextFieldKeyPressed
//TextFieldShouldReturn
