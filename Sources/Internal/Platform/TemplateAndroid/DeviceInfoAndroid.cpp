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

#if defined(__DAVAENGINE_ANDROID__)

#include "DeviceInfoAndroid.h"
#include "ExternC/AndroidLayer.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/JniHelpers.h"
#include <unistd.h>

namespace DAVA
{

JniDeviceInfo::JniDeviceInfo()
    : jniDeviceInfo("com/dava/framework/JNIDeviceInfo")
{
	getVersion = jniDeviceInfo.GetStaticMethod<jstring>("GetVersion");
	getManufacturer = jniDeviceInfo.GetStaticMethod<jstring>("GetManufacturer");
	getModel = jniDeviceInfo.GetStaticMethod<jstring>("GetModel");
	getLocale = jniDeviceInfo.GetStaticMethod<jstring>("GetLocale");
	getRegion = jniDeviceInfo.GetStaticMethod<jstring>("GetRegion");
	getTimeZone = jniDeviceInfo.GetStaticMethod<jstring>("GetTimeZone");
	getUDID = jniDeviceInfo.GetStaticMethod<jstring>("GetUDID");
	getName = jniDeviceInfo.GetStaticMethod<jstring>("GetName");
	getZBufferSize = jniDeviceInfo.GetStaticMethod<jint>("GetZBufferSize");
	getHTTPProxyHost = jniDeviceInfo.GetStaticMethod<jstring>("GetHTTPProxyHost");
	getHTTPNonProxyHosts = jniDeviceInfo.GetStaticMethod<jstring>("GetHTTPNonProxyHosts");
	getHTTPProxyPort = jniDeviceInfo.GetStaticMethod<jint>("GetHTTPProxyPort");
	getGPUFamily = jniDeviceInfo.GetStaticMethod<jint>("GetGPUFamily");
	getNetworkType = jniDeviceInfo.GetStaticMethod<jint>("GetNetworkType");
	getSignalStrength = jniDeviceInfo.GetStaticMethod<jint, jint>("GetSignalStrength");
	isPrimaryExternalStoragePresent = jniDeviceInfo.GetStaticMethod<jboolean>("IsPrimaryExternalStoragePresent");

}

namespace
{
    String jstringToString(jstring s)
    {
        DVASSERT(0 != s);
        String result;
        JNI::CreateStringFromJni(s, result);
        return result;
    }
}

String JniDeviceInfo::GetVersion()
{
	jstring s = getVersion();
	return jstringToString(s);
}

String JniDeviceInfo::GetManufacturer()
{
	jstring s = getManufacturer();
	return jstringToString(s);
}

String JniDeviceInfo::GetModel()
{
	jstring s = getModel();
	return jstringToString(s);
}

String JniDeviceInfo::GetLocale()
{
	jstring s = getLocale();
	return jstringToString(s);
}

String JniDeviceInfo::GetRegion()
{
	jstring s = getRegion();
	return jstringToString(s);
}

String JniDeviceInfo::GetTimeZone()
{
	jstring s = getTimeZone();
	return jstringToString(s);
}

String JniDeviceInfo::GetUDID()
{
	jstring s = getUDID();
	return jstringToString(s);
}

String JniDeviceInfo::GetName()
{
    jstring s = getName();
	return jstringToString(s);
}

int32 JniDeviceInfo::GetZBufferSize()
{
	return getZBufferSize();
}

String JniDeviceInfo::GetHTTPProxyHost()
{
	jstring s = getHTTPProxyHost();
	return jstringToString(s);
}

String JniDeviceInfo::GetHTTPNonProxyHosts()
{
	jstring s = getHTTPNonProxyHosts();
	return jstringToString(s);
}

int32 JniDeviceInfo::GetHTTPProxyPort()
{
	return getHTTPProxyPort();
}

int32 JniDeviceInfo::GetGPUFamily()
{
	return getGPUFamily();
}

int32 JniDeviceInfo::GetNetworkType()
{
	return getNetworkType();
}

int32 JniDeviceInfo::GetSignalStrength(int32 networkType)
{
	return getSignalStrength(networkType);
}

DeviceInfo::StorageInfo JniDeviceInfo::StorageInfoFromJava(jobject object)
{
	DeviceInfo::StorageInfo info;

	if (object)
	{
		JNIEnv *env = JNI::GetEnv();
		jclass classInfo = env->GetObjectClass(object);

		jfieldID fieldID;

		fieldID = env->GetFieldID(classInfo, "freeSpace", "J");
		info.freeSpace = env->GetLongField(object, fieldID);

		fieldID = env->GetFieldID(classInfo, "capacity", "J");
		info.totalSpace = env->GetLongField(object, fieldID);

		fieldID = env->GetFieldID(classInfo, "readOnly", "Z");
		info.readOnly = env->GetBooleanField(object, fieldID);

		fieldID = env->GetFieldID(classInfo, "removable", "Z");
		info.removable = env->GetBooleanField(object, fieldID);
		
		fieldID = env->GetFieldID(classInfo, "emulated", "Z");
		info.emulated = env->GetBooleanField(object, fieldID);

		fieldID = env->GetFieldID(classInfo, "path", "Ljava/lang/String;");
		jstring jStr = (jstring)env->GetObjectField(object, fieldID);
		char str[512] = {0};
		JNI::CreateStringFromJni(env, jStr, str);
		info.path = String(str);
	}

	return info;
}

DeviceInfo::StorageInfo JniDeviceInfo::GetInternalStorageInfo()
{
	JNIEnv *env = JNI::GetEnv();
	jmethodID mid = env->GetStaticMethodID(jniDeviceInfo, "GetInternalStorageInfo", "()Lcom/dava/framework/JNIDeviceInfo$StorageInfo;");

	DeviceInfo::StorageInfo info;

	if (mid)
	{
		jobject object = (jobject)env->CallStaticObjectMethod(jniDeviceInfo, mid);
		DAVA_JNI_EXCEPTION_CHECK
		if (object)
		{
			info = StorageInfoFromJava(object);
			info.type = DeviceInfo::STORAGE_TYPE_INTERNAL;
		}
	}

	return info;
}

bool JniDeviceInfo::IsPrimaryExternalStoragePresent()
{
	return isPrimaryExternalStoragePresent();
}

DeviceInfo::StorageInfo JniDeviceInfo::GetPrimaryExternalStorageInfo()
{
	DeviceInfo::StorageInfo info;
	if (!IsPrimaryExternalStoragePresent())
	{
		return info;
	}

	JNIEnv *env = JNI::GetEnv();

	jmethodID mid = env->GetStaticMethodID(jniDeviceInfo, "GetPrimaryExternalStorageInfo", "()Lcom/dava/framework/JNIDeviceInfo$StorageInfo;");

	if (mid)
	{
		jobject object = (jobject)env->CallStaticObjectMethod(jniDeviceInfo, mid);
		DAVA_JNI_EXCEPTION_CHECK
		if (object)
		{
			info = StorageInfoFromJava(object);
			info.type = DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL;
		}
	}

	return info;
}

List<DeviceInfo::StorageInfo> JniDeviceInfo::GetSecondaryExternalStoragesList()
{
	List<DeviceInfo::StorageInfo> list;

	JNIEnv *env = JNI::GetEnv();

	jmethodID mid = env->GetStaticMethodID(jniDeviceInfo, "GetSecondaryExternalStoragesList", "()[Lcom/dava/framework/JNIDeviceInfo$StorageInfo;");

	if (mid)
	{
		jarray array = (jarray)env->CallStaticObjectMethod(jniDeviceInfo, mid);
		DAVA_JNI_EXCEPTION_CHECK
		if (array)
		{
			jsize length = env->GetArrayLength(array);

			for (jsize i = 0; i < length; ++i)
			{
				jobject object = env->GetObjectArrayElement((jobjectArray)array, i);

				if (object)
				{
					DeviceInfo::StorageInfo info = StorageInfoFromJava(object);
					info.type = DeviceInfo::STORAGE_TYPE_SECONDARY_EXTERNAL;

					list.push_back(info);
				}
			}
		}
	}

	return list;
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

DeviceInfo::NetworkInfo DeviceInfo::GetNetworkInfo()
{
	DeviceInfo::NetworkInfo info;
	JniDeviceInfo jniDeviceInfo;
	info.networkType = (DeviceInfo::eNetworkType) jniDeviceInfo.GetNetworkType();
	info.signalStrength = jniDeviceInfo.GetSignalStrength(info.networkType);
	return info;
}

List<DeviceInfo::StorageInfo> DeviceInfo::GetStoragesList()
{
	JniDeviceInfo jniDeviceInfo;

	List<DeviceInfo::StorageInfo> l;

	StorageInfo internal = jniDeviceInfo.GetInternalStorageInfo();
	StorageInfo external = jniDeviceInfo.GetPrimaryExternalStorageInfo();
	List<DeviceInfo::StorageInfo> secondaryList = jniDeviceInfo.GetSecondaryExternalStoragesList();

	if (internal.type != DeviceInfo::STORAGE_TYPE_UNKNOWN)
	{
		l.push_back(internal);
	}
	if (external.type != DeviceInfo::STORAGE_TYPE_UNKNOWN)
	{
		l.push_back(external);
	}

	std::copy(secondaryList.begin(), secondaryList.end(), back_inserter(l));

    return l;
}

void DeviceInfo::InitializeScreenInfo()
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    screenInfo.width = core->GetViewWidth();
    screenInfo.height = core->GetViewHeight();
    screenInfo.scale = 1;
}


int32 DeviceInfo::GetCpuCount()
{
	return sysconf(_SC_NPROCESSORS_CONF);
}

}

#endif
