#include "Notification/LocalNotificationNotImplemented.h"

#if defined(__DAVAENGINE_WIN32__)

namespace DAVA
{
LocalNotificationNotImplemented::LocalNotificationNotImplemented(const String& _id)
{
    notificationId = _id;
}

LocalNotificationNotImplemented::~LocalNotificationNotImplemented()
{
}

void LocalNotificationNotImplemented::SetAction(const WideString& action)
{
}

void LocalNotificationNotImplemented::Hide()
{
}
void LocalNotificationNotImplemented::ShowText(const WideString& title, const WideString& text, bool useSound)
{
}
void LocalNotificationNotImplemented::ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound)
{
}

LocalNotificationImpl* LocalNotificationImpl::Create(const String& _id)
{
    return new LocalNotificationNotImplemented(_id);
}

void LocalNotificationNotImplemented::PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound)
{
}

void LocalNotificationNotImplemented::RemoveAllDelayedNotifications()
{
}
}
#endif
