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
#include "Platform/TemplateAndroid/JniHelpers.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

LocalNotificationAndroid::LocalNotificationAndroid(const String &_id)
	: notificationProvider("com/dava/framework/JNINotificationProvider")
{
    notificationId = _id;

    setText = notificationProvider.GetStaticMethod<void, jstring, jstring, jstring, jboolean>("NotifyText");
    setProgress = notificationProvider.GetStaticMethod<void, jstring, jstring, jstring, jint, jint, jboolean>("NotifyProgress");
    hideNotification = notificationProvider.GetStaticMethod<void, jstring>("HideNotification");
	enableTapAction = notificationProvider.GetStaticMethod<void, jstring>("EnableTapAction");

	notifyDelayed = notificationProvider.GetStaticMethod<void, jstring, jstring, jstring, jint, jboolean>("NotifyDelayed");
	removeAllDelayedNotifications = notificationProvider.GetStaticMethod<void>("RemoveAllDelayedNotifications");
}

void LocalNotificationAndroid::SetAction(const WideString &action)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
	JNIEnv *env = JNI::GetEnv();
    jstring jstrNotificationUid = env->NewStringUTF(notificationId.c_str());
    enableTapAction(jstrNotificationUid);
	env->DeleteLocalRef(jstrNotificationUid);
}

void LocalNotificationAndroid::Hide()
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
	JNIEnv *env = JNI::GetEnv();
    jstring jstrNotificationUid = env->NewStringUTF(notificationId.c_str());
    hideNotification(jstrNotificationUid);
    env->DeleteLocalRef(jstrNotificationUid);
}

void LocalNotificationAndroid::ShowText(const WideString &title, const WideString &text, bool useSound)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
	JNIEnv *env = JNI::GetEnv();

    jstring jstrNotificationUid = env->NewStringUTF(notificationId.c_str());

    jstring jStrTitle = JNI::ToJNIString(title);
    jstring jStrText = JNI::ToJNIString(text);

    setText(jstrNotificationUid, jStrTitle, jStrText, useSound);

    env->DeleteLocalRef(jstrNotificationUid);
	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}


void LocalNotificationAndroid::ShowProgress(const WideString &title, const WideString &text, const uint32 total, const uint32 progress, bool useSound)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
	JNIEnv *env = JNI::GetEnv();

    jstring jstrNotificationUid = env->NewStringUTF(notificationId.c_str());

    jstring jStrTitle = JNI::ToJNIString(title);
    jstring jStrText = JNI::ToJNIString(text);

    setProgress(jstrNotificationUid, jStrTitle, jStrText, total, progress, useSound);

    env->DeleteLocalRef(jstrNotificationUid);
	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}

void LocalNotificationAndroid::PostDelayedNotification(const WideString &title, const WideString &text, int delaySeconds, bool useSound)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);
	JNIEnv *env = JNI::GetEnv();

    jstring jstrNotificationUid = env->NewStringUTF(notificationId.c_str());
    jstring jStrTitle = JNI::ToJNIString(title);
    jstring jStrText = JNI::ToJNIString(text);

    notifyDelayed(jstrNotificationUid, jStrTitle, jStrText, delaySeconds, useSound);

    env->DeleteLocalRef(jstrNotificationUid);
    env->DeleteLocalRef(jStrTitle);
    env->DeleteLocalRef(jStrText);
}

void LocalNotificationAndroid::RemoveAllDelayedNotifications()
{
	removeAllDelayedNotifications();
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
