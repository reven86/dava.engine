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

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Utils/Utils.h"
#include "DeviceInfoAndroid.h"
#include "ExternC/AndroidLayer.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/JniHelpers.h"
#include <unistd.h>

namespace DAVA
{

DeviceInfoPrivate::DeviceInfoPrivate() : jniDeviceInfo("com/dava/framework/JNIDeviceInfo")
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

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return 	DeviceInfo::PLATFORM_ANDROID;
}

String DeviceInfoPrivate::GetPlatformString()
{
    return GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());
}

String DeviceInfoPrivate::GetVersion()
{
    return JNI::ToString(getVersion());
}

String DeviceInfoPrivate::GetManufacturer()
{
    return JNI::ToString(getManufacturer());
}

String DeviceInfoPrivate::GetModel()
{
    return JNI::ToString(getModel());
}

String DeviceInfoPrivate::GetLocale()
{
    return JNI::ToString(getLocale());
}

String DeviceInfoPrivate::GetRegion()
{
    return JNI::ToString(getRegion());
}

String DeviceInfoPrivate::GetTimeZone()
{
    return JNI::ToString(getTimeZone());
}

String DeviceInfoPrivate::GetUDID()
{
    return JNI::ToString(getUDID());
}

WideString DeviceInfoPrivate::GetName()
{
    return StringToWString(JNI::ToString(getName()));
}

int32 DeviceInfoPrivate::GetZBufferSize()
{
	return static_cast<int32>(getZBufferSize());
}

String DeviceInfoPrivate::GetHTTPProxyHost()
{
    return JNI::ToString(getHTTPProxyHost());
}

String DeviceInfoPrivate::GetHTTPNonProxyHosts()
{
    return JNI::ToString(getHTTPNonProxyHosts());
}

int32 DeviceInfoPrivate::GetHTTPProxyPort()
{
	return static_cast<int32>(getHTTPProxyPort());
}

DeviceInfo::ScreenInfo& DeviceInfoPrivate::GetScreenInfo()
{
    return screenInfo;
}

eGPUFamily DeviceInfoPrivate::GetGPUFamily()
{
	return static_cast<eGPUFamily>(getGPUFamily());
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    DeviceInfo::NetworkInfo info;
    info.networkType = static_cast<DeviceInfo::eNetworkType>(GetNetworkType());
    info.signalStrength = GetSignalStrength(info.networkType);
    return info;
}

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetStoragesList()
{
    List<DeviceInfo::StorageInfo> l;

    DeviceInfo::StorageInfo internal = GetInternalStorageInfo();
    DeviceInfo::StorageInfo external = GetPrimaryExternalStorageInfo();
    List<DeviceInfo::StorageInfo> secondaryList = GetSecondaryExternalStoragesList();

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

void DeviceInfoPrivate::InitializeScreenInfo()
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    screenInfo.width = core->GetViewWidth();
    screenInfo.height = core->GetViewHeight();
    screenInfo.scale = 1;
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType type)
{
    //TODO: remove this empty realization and implement detection of HID connection
    return false;
}

bool DeviceInfoPrivate::IsTouchPresented()
{
    //TODO: remove this empty realization and implement detection touch
    return true;
}

DeviceInfo::StorageInfo DeviceInfoPrivate::StorageInfoFromJava(jobject object)
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

        info.path = JNI::ToString(jStr);
    }

    return info;
}

int32 DeviceInfoPrivate::GetNetworkType()
{
    return getNetworkType();
}

int32 DeviceInfoPrivate::GetSignalStrength(int32 networkType)
{
    return getSignalStrength(networkType);
}

DeviceInfo::StorageInfo DeviceInfoPrivate::GetInternalStorageInfo()
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

bool DeviceInfoPrivate::IsPrimaryExternalStoragePresent()
{
	return isPrimaryExternalStoragePresent();
}

DeviceInfo::StorageInfo DeviceInfoPrivate::GetPrimaryExternalStorageInfo()
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

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetSecondaryExternalStoragesList()
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

}

#endif
