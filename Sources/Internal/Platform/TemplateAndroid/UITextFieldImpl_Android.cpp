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

#include "UI/UITextFieldImpl.h"
#include "UI/UITextField.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{
static uint32_t sId = 0;
static DAVA::Map<uint32_t, UITextFieldImpl_Android*> idToImpl;

UITextFieldImpl_Android* GetUITextFieldImpl(uint32_t id)
{
    DAVA::Map<uint32_t, UITextFieldImpl_Android*>::iterator iter = idToImpl.find(id);
    if (iter != idToImpl.end())
        return iter->second;

    return NULL;
}

jclass UITextFieldImpl_Android::gJavaClass = NULL;
const char* UITextFieldImpl_Android::gJavaClassName = NULL;

jclass UITextFieldImpl_Android::GetJavaClass() const
{
	return gJavaClass;
}

const char* UITextFieldImpl_Android::GetJavaClassName() const
{
	return gJavaClassName;
}

UITextFieldImpl_Android::UITextFieldImpl_Android(UITextField * tf)
    : UITextFieldImpl(tf)
{
    id = sId++;
    idToImpl[id] = this;
}

UITextFieldImpl_Android::~UITextFieldImpl_Android()
{
    idToImpl.erase(id);
}

void UITextFieldImpl_Android::Create(const Rect &controlRect)
{
	Rect rect = V2P(controlRect);
	jmethodID mid = GetMethodID("Create", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				rect.x,
				rect.y,
				rect.dx,
				rect.dy);
	}
}

void UITextFieldImpl_Android::Destroy()
{
	jmethodID mid = GetMethodID("Destroy", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id);
	}
}

void UITextFieldImpl_Android::UpdateRect(const Rect & controlRect)
{
    if (controlRect == rect)
        return;
    rect = controlRect;
	Rect newRect = V2P(controlRect);
	jmethodID mid = GetMethodID("UpdateRect", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				newRect.x,
				newRect.y,
				newRect.dx,
				newRect.dy);
	}
}
void UITextFieldImpl_Android::GetText(WideString &text)
{
	jmethodID mid = GetMethodID("GetText", "(I)Ljava/lang/String;");
	if (mid)
	{
		jstring string = (jstring)GetEnvironment()->CallStaticObjectMethod(
				GetJavaClass(),
				mid,
				id);
		const char * utf8str = GetEnvironment()->GetStringUTFChars(string, NULL);
		jsize utf8strLen = env->GetStringUTFLength(string);
		UTF8Utils::EncodeToWideString((uint8*)utf8str, utf8strLen, text);
		GetEnvironment()->ReleaseStringUTFChars(string, utf8str);
		GetEnvironment()->DeleteLocalRef(string);
	}
}
void UITextFieldImpl_Android::SetText(const WideString & newText)
{
	text = newText;
    String utfText = UTF8Utils::EncodeToUTF8(text);
	jmethodID mid = GetMethodID("SetText", "(ILjava/lang/String;)V");
	if (mid)
	{
		jstring jStrDefaultText = GetEnvironment()->NewStringUTF(utfText.c_str());
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				jStrDefaultText);
		GetEnvironment()->DeleteLocalRef(jStrDefaultText);
	}
}

void UITextFieldImpl_Android::SetTextColor(float r, float g, float b, float a)
{
	jmethodID mid = GetMethodID("SetTextColor", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				r,
				g,
				b,
				a);
	}
}

void UITextFieldImpl_Android::SetFontSize(float size)
{
	jmethodID mid = GetMethodID("SetFontSize", "(IF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				size);
	}
}

void UITextFieldImpl_Android::SetIsPassword(bool isPassword)
{
	jmethodID mid = GetMethodID("SetIsPassword", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				isPassword);
	}
}

void UITextFieldImpl_Android::SetTextAlign(int32_t align)
{
	jmethodID mid = GetMethodID("SetTextAlign", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				align);
	}
}

void UITextFieldImpl_Android::SetInputEnabled(bool value)
{
	jmethodID mid = GetMethodID("SetInputEnabled", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetAutoCapitalizationType(int32_t value)
{
	jmethodID mid = GetMethodID("SetAutoCapitalizationType", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetAutoCorrectionType(int32_t value)
{
	jmethodID mid = GetMethodID("SetAutoCorrectionType", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetSpellCheckingType(int32_t value)
{
	jmethodID mid = GetMethodID("SetSpellCheckingType", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetKeyboardAppearanceType(int32_t value)
{
	jmethodID mid = GetMethodID("SetKeyboardAppearanceType", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetKeyboardType(int32_t value)
{
	jmethodID mid = GetMethodID("SetKeyboardType", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetReturnKeyType(int32_t value)
{
	jmethodID mid = GetMethodID("SetReturnKeyType", "(II)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::SetEnableReturnKeyAutomatically(bool value)
{
	jmethodID mid = GetMethodID("SetEnableReturnKeyAutomatically", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id,
				value);
	}
}

void UITextFieldImpl_Android::HideField()
{
	jmethodID mid = GetMethodID("HideField", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id);
	}
}

void UITextFieldImpl_Android::ShowField()
{
	jmethodID mid = GetMethodID("ShowField", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id);
	}
}

void UITextFieldImpl_Android::OpenKeyboard()
{
	jmethodID mid = GetMethodID("OpenKeyboard", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id);
	}
}

void UITextFieldImpl_Android::CloseKeyboard()
{
	jmethodID mid = GetMethodID("CloseKeyboard", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
				GetJavaClass(),
				mid,
				id);
	}
}

uint32 UITextFieldImpl_Android::GetCursorPos()
{
	jmethodID mid = GetMethodID("GetCursorPos", "(I)I");
	if (!mid)
		return 0;

	return GetEnvironment()->CallStaticIntMethod(GetJavaClass(), mid, id);
}

void UITextFieldImpl_Android::SetCursorPos(uint32 pos)
{
	jmethodID mid = GetMethodID("SetCursorPos", "(II)V");
	if (!mid)
		return;
	GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, id, pos);
}

bool UITextFieldImpl_Android::TextFieldKeyPressed(uint32_t id, int32 replacementLocation, int32 replacementLength, const WideString &text)
{
    UITextFieldImpl* impl = GetUITextFieldImpl(id)->textFieldImpl;
    if (!impl)
        return true;

    UITextField * control = impl->GetTextFieldControl();

    if(!control || !control->GetDelegate())
    	return true;

    return control->GetDelegate()->TextFieldKeyPressed(control, replacementLocation, replacementLength, text);
}

void UITextFieldImpl_Android::TextFieldShouldReturn(uint32_t id)
{
    UITextFieldImpl* impl = GetUITextFieldImpl(id)->textFieldImpl;
    if (!impl)
        return;

    UITextField * control = impl->GetTextFieldControl();

    if(!control || !control->GetDelegate())
    	return;

    control->GetDelegate()->TextFieldShouldReturn( control );
}
};
#endif //#if defined(__DAVAENGINE_ANDROID__)
