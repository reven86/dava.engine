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



#include "NotificationAndroid.h"
#include "Platform/Notification.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

using namespace DAVA;

jclass JniNotification::gJavaClass = NULL;
const char* JniNotification::gJavaClassName = NULL;

jclass JniNotification::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniNotification::GetJavaClassName() const
{
	return gJavaClassName;
}

void JniNotification::ShowNotifitaionWithProgress(uint32 id,
			const WideString& title,
			const WideString& text,
			int32 maxValue,
			int32 value)
{
	jmethodID mid = GetMethodID("ShowNotifitaionWithProgress", "(ILjava/lang/String;Ljava/lang/String;II)V");
	if (mid)
	{
		jstring jStrTitle = CreateJString(GetEnvironment(), title);
		jstring jStrText = CreateJString(GetEnvironment(), text);
		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id,
						jStrTitle,
						jStrText,
						maxValue,
						value);
		GetEnvironment()->DeleteLocalRef(jStrTitle);
		GetEnvironment()->DeleteLocalRef(jStrText);
	}
}

void JniNotification::HideNotification(uint32 id)
{
	jmethodID mid = GetMethodID("HideNotification", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id);
	}
}

void Notification::ShowNotifitaion(uint32 id,
			const WideString& title,
			const WideString& text)
{
	ShowNotifitaionWithProgress(id, title, text, 0, 0);
}

void Notification::ShowNotifitaionWithProgress(uint32 id,
			const WideString& title,
			const WideString& text,
			int32 maxValue,
			int32 value)
{
	JniNotification jni;
	jni.ShowNotifitaionWithProgress(id, title, text, maxValue, value);
}

void Notification::HideNotification(uint32 id)
{
	JniNotification jni;
	jni.HideNotification(id);
}

#endif
