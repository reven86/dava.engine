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

#include "Notification/LocalNotificationAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Notification/LocalNotificationController.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "Thread/LockGuard.h"

namespace DAVA
{

jclass LocalNotificationAndroid::gJavaClass = NULL;
const char* LocalNotificationAndroid::gJavaClassName = NULL;

LocalNotificationAndroid::LocalNotificationAndroid(const String &_id)
	: methodSetText(0)
	, methodSetProgress(0)
{
    notificationId = _id;

	SetJavaClass(GetEnvironment(), "com/dava/framework/JNINotificationProvider", &LocalNotificationAndroid::gJavaClass, &LocalNotificationAndroid::gJavaClassName);
}

jclass LocalNotificationAndroid::GetJavaClass() const
{
	return gJavaClass;
}

const char* LocalNotificationAndroid::GetJavaClassName() const
{
	return gJavaClassName;
}

void LocalNotificationAndroid::SetAction(const WideString &action)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
    jstring jstrNotificationUid = GetEnvironment()->NewStringUTF(notificationId.c_str());

	GetEnvironment()->CallStaticVoidMethod(
					gJavaClass,
					GetMethodID("EnableTapAction", "(Ljava/lang/String;)V"),
					jstrNotificationUid);
    GetEnvironment()->DeleteLocalRef(jstrNotificationUid);
}

void LocalNotificationAndroid::Hide()
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
    jstring jstrNotificationUid = GetEnvironment()->NewStringUTF(notificationId.c_str());

	GetEnvironment()->CallStaticVoidMethod(
					gJavaClass,
					GetMethodID("HideNotification", "(Ljava/lang/String;)V"),
					jstrNotificationUid);
    GetEnvironment()->DeleteLocalRef(jstrNotificationUid);
}

void LocalNotificationAndroid::ShowText(const WideString &title, const WideString text)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
    jstring jstrNotificationUid = GetEnvironment()->NewStringUTF(notificationId.c_str());

	JNIEnv *env = GetEnvironment();

	jstring jStrTitle = CreateJString(env, title);
	jstring jStrText = CreateJString(env, text);

	static const jmethodID methodSetText = GetMethodID("NotifyText", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	DVASSERT(0 != methodSetText);

	env->CallStaticVoidMethod(
					gJavaClass,
					methodSetText,
					jstrNotificationUid,
					jStrTitle,
					jStrText);

    env->DeleteLocalRef(jstrNotificationUid);
	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}


void LocalNotificationAndroid::ShowProgress(const WideString &title, const WideString text, const uint32 total, const uint32 progress)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
    jstring jstrNotificationUid = GetEnvironment()->NewStringUTF(notificationId.c_str());

	JNIEnv *env = GetEnvironment();

	jstring jStrTitle = CreateJString(env, title);
	jstring jStrText = CreateJString(env, text);

	static const jmethodID methodSetProgress = GetMethodID("NotifyProgress", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)V");
	DVASSERT(0 != methodSetProgress);

	env->CallStaticVoidMethod(
					gJavaClass,
					methodSetProgress,
					jstrNotificationUid,
					jStrTitle,
					jStrText,
					total,
					progress);

    env->DeleteLocalRef(jstrNotificationUid);
	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}

void LocalNotificationAndroid::PostDelayedNotification(const WideString &title, const WideString &text, int delaySeconds)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
	JNIEnv *env = GetEnvironment();
    jstring jstrNotificationUid = env->NewStringUTF(notificationId.c_str());
	jstring jStrTitle = CreateJString(env, title);
	jstring jStrText = CreateJString(env, text);

	static const jmethodID methodNotifyDelayed = GetMethodID("NotifyDelayed", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
	DVASSERT(0 != methodNotifyDelayed);

	env->CallStaticVoidMethod(
					gJavaClass,
					methodNotifyDelayed,
					jstrNotificationUid,
					jStrTitle,
					jStrText,
					delaySeconds);

    env->DeleteLocalRef(jstrNotificationUid);
	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}

void LocalNotificationAndroid::RemoveAllDelayedNotifications()
{
	JNIEnv *env = GetEnvironment();
	static const jmethodID methodRemoveAllDelayedNotifications = GetMethodID("RemoveAllDelayedNotifications", "()V");
	DVASSERT(0 != methodRemoveAllDelayedNotifications);

	env->CallStaticVoidMethod(
					gJavaClass,
					methodRemoveAllDelayedNotifications);
}

LocalNotificationImpl *LocalNotificationImpl::Create(const String &_id)
{
    return new LocalNotificationAndroid(_id);
}

}

extern "C"
{
	void Java_com_dava_framework_JNINotificationProvider_onNotificationPressed(JNIEnv* env, jobject classthis, jstring uid)
	{
        const char *str = env->GetStringUTFChars(uid, 0);
		DAVA::LocalNotificationController::Instance()->OnNotificationPressed(DAVA::String(str));
        env->ReleaseStringUTFChars(uid, str);
	}
}

#endif
