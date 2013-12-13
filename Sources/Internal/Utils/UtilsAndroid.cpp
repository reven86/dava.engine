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


#include "Utils/Utils.h"
#include "UtilsAndroid.h"

using namespace DAVA;

#if defined(__DAVAENGINE_ANDROID__)

jclass JniUtils::gJavaClass = NULL;
const char* JniUtils::gJavaClassName = NULL;

jclass JniUtils::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniUtils::GetJavaClassName() const
{
	return gJavaClassName;
}

bool JniUtils::DisableSleepTimer()
{
	jmethodID mid = GetMethodID("DisableSleepTimer", "()V");
	if (!mid)
		return false;

	GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid);
	return true;
}

bool JniUtils::EnableSleepTimer()
{
	jmethodID mid = GetMethodID("EnableSleepTimer", "()V");
	if (!mid)
		return false;

	GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid);
	return true;
}

bool JniUtils::IsFile(const String& absolutePath)
{
	jmethodID mid = GetMethodID("IsFile", "(Ljava/lang/String;)Z");
	if (!mid)
		return false;

	jstring jAbsolutePath = GetEnvironment()->NewStringUTF(absolutePath.c_str());
	bool res = GetEnvironment()->CallStaticBooleanMethod(GetJavaClass(), mid, jAbsolutePath);
	GetEnvironment()->DeleteLocalRef(jAbsolutePath);
	return res;
}

bool JniUtils::IsDirectory(const String& absolutePath)
{
	jmethodID mid = GetMethodID("IsDirectory", "(Ljava/lang/String;)Z");
	if (!mid)
		return false;

	jstring jAbsolutePath = GetEnvironment()->NewStringUTF(absolutePath.c_str());
	bool res = GetEnvironment()->CallStaticBooleanMethod(GetJavaClass(), mid, jAbsolutePath);
	GetEnvironment()->DeleteLocalRef(jAbsolutePath);
	return res;
}



void DAVA::DisableSleepTimer()
{
	JniUtils jniUtils;
	jniUtils.DisableSleepTimer();
}

void DAVA::EnableSleepTimer()
{
	JniUtils jniUtils;
	jniUtils.EnableSleepTimer();
}

uint64 DAVA::EglGetCurrentContext()
{
	//TODO: in case if context checking will ever be needed on Android,
	//TODO: implement this method similar to any other platform
	//TODO: it should return uint64 representation of the current OpenGL context
	//TDOD: see iOS implementation for example
	DVASSERT(false && "Implement this method for Android if needed");
	return 0;
}

#endif //#if defined(__DAVAENGINE_ANDROID__)
