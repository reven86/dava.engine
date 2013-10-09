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



#include "Network/MailSender.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Utils/StringFormat.h"
#include <shellapi.h>

namespace DAVA
{

bool MailSender::SendEmail(const WideString& email, const WideString& subject, const WideString& messageText)
{
	// Don't try to send email if recipient is not set
	if (email.empty() || email == L"")
		return false;
	WideString mailtoText = L"";
	
	// Create mailto text
	mailtoText = Format(L"mailto:?to=%s&subject=%s&body=%s", email.c_str(), subject.c_str(), messageText.c_str());
	// Call default mail client
	int result = (int)ShellExecute(NULL, L"open", mailtoText.c_str(), NULL, NULL, SW_SHOWNORMAL);
	
	// If the function succeeds, it returns a value greater than 32.
	// If the function fails, it returns an error value that indicates the cause of the failure. 
	return (result > 32);
}

};

#elif defined(__DAVAENGINE_ANDROID__)
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{

jclass JniMailSender::gJavaClass = NULL;
const char* JniMailSender::gJavaClassName = NULL;

jclass JniMailSender::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniMailSender::GetJavaClassName() const
{
	return gJavaClassName;
}

bool JniMailSender::SendEmail(const String& email, const String& subject, const String& messageText)
{
	jmethodID mid = GetMethodID("SendEMail", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
	bool res = false;
	if (mid)
	{
		jstring jEMail = GetEnvironment()->NewStringUTF(email.c_str());
		jstring jSubject = GetEnvironment()->NewStringUTF(subject.c_str());
		jstring jMessageText = GetEnvironment()->NewStringUTF(messageText.c_str());
		res = GetEnvironment()->CallStaticBooleanMethod(GetJavaClass(), mid, jEMail, jSubject, jMessageText);
		GetEnvironment()->DeleteLocalRef(jEMail);
		GetEnvironment()->DeleteLocalRef(jSubject);
		GetEnvironment()->DeleteLocalRef(jMessageText);
	}

	return res;
}

bool MailSender::SendEmail(const WideString& email, const WideString& subject, const WideString& messageText)
{
	JniMailSender jniMailSender;

	return jniMailSender.SendEmail(UTF8Utils::EncodeToUTF8(email), UTF8Utils::EncodeToUTF8(subject), UTF8Utils::EncodeToUTF8(messageText));
}

};

#endif
