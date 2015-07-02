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


#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_ANDROID_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_ANDROID_H__

#include "Base/BaseTypes.h"

#if defined (__DAVAENGINE_ANDROID__)

#include "Notification/LocalNotificationImpl.h"
#include "Platform/TemplateAndroid/JniExtensions.h"
#include "Base/Message.h"
#include "Concurrency/Mutex.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{

class LocalNotificationAndroid: public LocalNotificationImpl
{
public:
	LocalNotificationAndroid(const String &_id);
	virtual void SetAction(const WideString &action);
	virtual void Hide();
	virtual void ShowText(const WideString &title, const WideString &text, const bool useSound);
	virtual void ShowProgress(const WideString &title, const WideString &text, const uint32 total, const uint32 progress, bool useSound);
    virtual void PostDelayedNotification(WideString const &title, WideString const &text, int delaySeconds, bool useSound);
    virtual void RemoveAllDelayedNotifications();

private:
	Mutex javaCallMutex;
	JNI::JavaClass notificationProvider;

	Function<void (jstring, jstring, jstring, jboolean)> setText;
	Function<void (jstring, jstring, jstring, jint, jint, jboolean)> setProgress;
	Function<void (jstring)> hideNotification;
	Function<void (jstring)> enableTapAction;
	Function<void (jstring, jstring, jstring, jint, jboolean)> notifyDelayed;
	Function<void ()> removeAllDelayedNotifications;
};

}
#endif //__DAVAENGINE_ANDROID__

#endif // __NOTIFICATION_ANDROID_H__
