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


#include "Platform/DateTime.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"
#include "FileSystem/LocalizationSystem.h"


#if defined(__DAVAENGINE_ANDROID__)
#include "DateTimeAndroid.h"
#include "ExternC/AndroidLayer.h"

namespace DAVA
{

JniDateTime::JniDateTime()
    : jniDateTime("com/dava/framework/JNIDateTime")
{
    getTimeAsString = jniDateTime.GetStaticMethod<jstring, jstring, jstring, jlong, jint>("GetTimeAsString");
    getLocalTimeZoneOffset = jniDateTime.GetStaticMethod<jint>("GetLocalTimeZoneOffset");
}

WideString JniDateTime::AsWString(const WideString& format, const String& countryCode, long timeStamp, int tzOffset)
{
    JNIEnv *env = JNI::GetEnv();

    jstring jFormat = env->NewStringUTF(UTF8Utils::EncodeToUTF8(format).c_str());
    jstring jCountryCode = env->NewStringUTF(countryCode.c_str());

    jstring obj = getTimeAsString(jFormat, jCountryCode, (long long)timeStamp, tzOffset);

    env->DeleteLocalRef(jFormat);
    env->DeleteLocalRef(jCountryCode);

    WideString result = JNI::ToWideString(obj);
    return result;
}

int JniDateTime::GetLocalTimeZoneOffset()
{
    return getLocalTimeZoneOffset();
}

WideString DateTime::AsWString(const wchar_t* format) const
{
	JniDateTime jniDateTime;
	String countryCode = LocalizationSystem::Instance()->GetCountryCode();
	WideString retString = jniDateTime.AsWString( WideString (format), countryCode, innerTime, timeZoneOffset);

	return retString;
}

WideString DateTime::GetLocalizedDate() const
{
    return AsWString(L"%x");
}

WideString DateTime::GetLocalizedTime() const
{
    return AsWString(L"%X");
}

int32 DateTime::GetLocalTimeZoneOffset()
{
	JniDateTime jniDateTime;
	return jniDateTime.GetLocalTimeZoneOffset();
}

}

#endif
