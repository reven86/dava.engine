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


#include "Notification/LocalNotificationController.h"
#include "Notification/LocalNotificationProgress.h"
#include "Notification/LocalNotificationText.h"
#include "Notification/LocalNotificationDelayed.h"

#include "Base/BaseTypes.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

LocalNotificationController::~LocalNotificationController()
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
            LocalNotification *notification = (*it);
            it = notificationsList.erase(it);
            SafeRelease(notification);
        }
    }
}

LocalNotificationProgress *const LocalNotificationController::CreateNotificationProgress(const WideString &title, const WideString &text, uint32 maximum, uint32 current, bool useSound)
{
	LocalNotificationProgress *note = new LocalNotificationProgress();
    
    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetProgressCurrent(current);
        note->SetProgressTotal(maximum);
        note->SetUseSound(useSound);
        note->SetAction(Message());

        LockGuard<Mutex> guard(notificationsListMutex);
        notificationsList.push_back(note);
    }

    return note;
}

LocalNotificationText *const LocalNotificationController::CreateNotificationText(const WideString &title, const WideString &text, bool useSound)
{
	LocalNotificationText *note = new LocalNotificationText();

    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetAction(Message());
        note->SetUseSound(useSound);

        LockGuard<Mutex> guard(notificationsListMutex);
        notificationsList.push_back(note);
    }

    return note;
}

bool LocalNotificationController::Remove(LocalNotification *notification)
{
	LockGuard<Mutex> guard(notificationsListMutex);

    auto endIt = end(notificationsList);
    auto it = find(begin(notificationsList), endIt, notification);
    if (endIt != it)
    {
        (*it)->Release();
        notificationsList.erase(it);
        return true;
    }
    return false;
}

bool LocalNotificationController::RemoveById(const String &notificationId)
{
    LockGuard<Mutex> guard(notificationsListMutex);

    auto endIt = end(notificationsList);
    auto it = find_if(begin(notificationsList), endIt, [&notificationId](LocalNotification* note)->bool
    {
        return 0 == note->GetId().compare(notificationId);
    });
    
    if (it != endIt)
    {
        (*it)->Release();
        notificationsList.erase(it);
        return true;
    }

    return false;
}

void LocalNotificationController::Clear()
{
    LockGuard<Mutex> guard(notificationsListMutex);
    for (auto note : notificationsList)
    {
        note->Release();
    }
    notificationsList.clear();
}
    
void LocalNotificationController::Update()
{
	LockGuard<Mutex> guard(notificationsListMutex);

    for (auto notification : notificationsList)
    {
        notification->Update();
    }
}

LocalNotification *const LocalNotificationController::GetNotificationById(const String &id)
{
	LockGuard<Mutex> guard(notificationsListMutex);
    for (auto notification : notificationsList)
    {
        if (notification->GetId().compare(id) == 0)
        {
            return notification;
        }
    }

    return NULL;
}

void LocalNotificationController::OnNotificationPressed(const String &id)
{
	LocalNotification *const notification = GetNotificationById(id);
	if (NULL != notification)
	{
		notification->RunAction();
	}
}

void LocalNotificationController::PostDelayedNotification(const WideString &title, const WideString &text, int delaySeconds, bool useSound)
{
    LocalNotificationDelayed *notification = new LocalNotificationDelayed();
    notification->SetTitle(title);
    notification->SetText(text);
    notification->SetDelaySeconds(delaySeconds);
    notification->SetUseSound(useSound);
    notification->Post();
    SafeRelease(notification);
}

void LocalNotificationController::RemoveAllDelayedNotifications()
{
    LocalNotificationDelayed *notification = new LocalNotificationDelayed();
    notification->RemoveAllDelayedNotifications();
    SafeRelease(notification);
}

}
