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



#include "../../Platform/DeviceInfo.h"
#include "../../Utils/Utils.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "DeviceInfoAndroid.h"
#include "ExternC/AndroidLayer.h"

DAVA::String intermediateStr;

extern "C"
{

void Java_com_dava_framework_JNIDeviceInfo_SetJString(JNIEnv* env, jobject classthis, jstring jString)
{
	char str[256] = {0};
	CreateStringFromJni(env, jString, str);
	intermediateStr = DAVA::String(str);
}

}

namespace DAVA
{

jclass JniDeviceInfo::gJavaClass = NULL;
const char* JniDeviceInfo::gJavaClassName = NULL;

jclass JniDeviceInfo::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniDeviceInfo::GetJavaClassName() const
{
	return gJavaClassName;
}

String JniDeviceInfo::GetVersion()
{
	intermediateStr = "";
	jmethodID mid = GetMethodID("GetVersion", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetManufacturer()
{
	intermediateStr = "";
	jmethodID mid = GetMethodID("GetManufacturer", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetModel()
{
	intermediateStr = "";

	jmethodID mid = GetMethodID("GetModel", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetLocale()
{
	intermediateStr = "";

	jmethodID mid = GetMethodID("GetLocale", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetRegion()
{
	intermediateStr = "";
	jmethodID mid = GetMethodID("GetRegion", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetTimeZone()
{
	intermediateStr = "";
	jmethodID mid = GetMethodID("GetTimeZone", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetUDID()
{
	intermediateStr = "";
	jmethodID mid = GetMethodID("GetUDID", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

String JniDeviceInfo::GetName()
{
	intermediateStr = "";
	jmethodID mid = GetMethodID("GetName", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, 0);

	return intermediateStr;
}

int JniDeviceInfo::GetZBufferSize()
{
	jmethodID mid = GetMethodID("GetZBufferSize", "()I");
	if (mid)
		return GetEnvironment()->CallStaticIntMethod(GetJavaClass(), mid);

	return 0;
}

String JniDeviceInfo::GetHTTPProxyHost()
{
	jmethodID mid = GetMethodID("GetHTTPProxyHost", "()Ljava/lang/String;");
	String returnStr = "";

	if (mid)
	{
		jobject obj = GetEnvironment()->CallStaticObjectMethod(GetJavaClass(), mid);
		CreateStringFromJni(env, jstring(obj), returnStr);
	}

	return returnStr;
}

String JniDeviceInfo::GetHTTPNonProxyHosts()
{
	jmethodID mid = GetMethodID("GetHTTPNonProxyHosts", "()Ljava/lang/String;");
	String returnStr = "";

	if (mid)
	{
		jobject obj = GetEnvironment()->CallStaticObjectMethod(GetJavaClass(), mid);
		CreateStringFromJni(env, jstring(obj), returnStr);
	}

	return returnStr;
}

int JniDeviceInfo::GetHTTPProxyPort()
{
	jmethodID mid = GetMethodID("GetHTTPProxyPort", "()I");
	if (mid)
	{
		return GetEnvironment()->CallStaticIntMethod(GetJavaClass(), mid);
	}

	return 0;
}

int JniDeviceInfo::GetGPUFamily()
{
	jmethodID mid = GetMethodID("GetGPUFamily", "()I");
	if (mid)
		return GetEnvironment()->CallStaticIntMethod(GetJavaClass(), mid);

	return -1;
}

String DeviceInfo::GetVersion()
{
	JniDeviceInfo jniDeviceInfo;
	String version = jniDeviceInfo.GetVersion();

	return version;
}

String DeviceInfo::GetManufacturer()
{
	JniDeviceInfo jniDeviceInfo;
	String version = jniDeviceInfo.GetManufacturer();

	return version;
}

String DeviceInfo::GetModel()
{
	JniDeviceInfo jniDeviceInfo;
	String version = jniDeviceInfo.GetModel();

	return version;
}

String DeviceInfo::GetLocale()
{
	JniDeviceInfo jniDeviceInfo;
	String version = jniDeviceInfo.GetLocale();

	return version;
}

String DeviceInfo::GetRegion()
{
	JniDeviceInfo jniDeviceInfo;
	String version = jniDeviceInfo.GetRegion();

	return version;
}

String DeviceInfo::GetTimeZone()
{
	JniDeviceInfo jniDeviceInfo;
	String version = jniDeviceInfo.GetTimeZone();

	return version;
}

String DeviceInfo::GetUDID()
{
	JniDeviceInfo jniDeviceInfo;
	String udid = jniDeviceInfo.GetUDID();

	return udid;
}

WideString DeviceInfo::GetName()
{
	JniDeviceInfo jniDeviceInfo;
	String name = jniDeviceInfo.GetName();

	return StringToWString(name);
}

int DeviceInfo::GetZBufferSize()
{
	JniDeviceInfo jniDeviceInfo;
	return jniDeviceInfo.GetZBufferSize();
}

String DeviceInfo::GetHTTPProxyHost()
{
	JniDeviceInfo jniDeviceInfo;
	return jniDeviceInfo.GetHTTPProxyHost();
}

String DeviceInfo::GetHTTPNonProxyHosts()
{
	JniDeviceInfo jniDeviceInfo;
	return jniDeviceInfo.GetHTTPNonProxyHosts();
}

int DeviceInfo::GetHTTPProxyPort()
{
	JniDeviceInfo jniDeviceInfo;
	return jniDeviceInfo.GetHTTPProxyPort();
}

eGPUFamily DeviceInfo::GetGPUFamily()
{
	JniDeviceInfo jniDeviceInfo;
	return (eGPUFamily) jniDeviceInfo.GetGPUFamily();
}

}

#endif
