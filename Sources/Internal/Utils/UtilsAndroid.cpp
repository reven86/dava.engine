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

JniUtils::JniUtils()
    : jniUtils("com/dava/framework/JNIUtils")
{
	disableSleepTimer = jniUtils.GetStaticMethod<void>("DisableSleepTimer");
	enableSleepTimer = jniUtils.GetStaticMethod<void>("EnableSleepTimer");
	openURL = jniUtils.GetStaticMethod<void, jstring>("OpenURL");
	generateGUID = jniUtils.GetStaticMethod<jstring>("GenerateGUID");
}

bool JniUtils::DisableSleepTimer()
{
	disableSleepTimer();
	return true;
}

bool JniUtils::EnableSleepTimer()
{
	enableSleepTimer();
	return true;
}

void JniUtils::OpenURL(const String& url)
{
	JNIEnv *env = JNI::GetEnv();
	jstring jUrl = env->NewStringUTF(url.c_str());
	openURL(jUrl);
	env->DeleteLocalRef(jUrl);

}

String JniUtils::GenerateGUID()
{
    JNIEnv *env = JNI::GetEnv();
    jstring jstr = generateGUID();
    const char *str = env->GetStringUTFChars(jstr, 0);
    DAVA::String result(str);
    env->ReleaseStringUTFChars(jstr, str);
    return result;
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

void DAVA::OpenURL(const String& url)
{
	JniUtils jniUtils;
	jniUtils.OpenURL(url);
}

String DAVA::GenerateGUID()
{
    JniUtils jniUtils;
    return jniUtils.GenerateGUID();
}

#endif //#if defined(__DAVAENGINE_ANDROID__)
