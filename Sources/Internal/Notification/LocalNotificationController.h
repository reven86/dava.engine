#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_CONTROLLER_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_CONTROLLER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Message.h"
#include "Concurrency/Mutex.h"
#include "Notification/LocalNotificationAndroid.h"
#include "Notification/LocalNotificationNotImplemented.h"

namespace DAVA
{
class LocalNotification;
class LocalNotificationText;
class LocalNotificationProgress;

class LocalNotificationController : public Singleton<LocalNotificationController>
{
    friend class LocalNotification;

public:
    virtual ~LocalNotificationController();
    LocalNotificationProgress* const CreateNotificationProgress(const WideString& title = L"", const WideString& text = L"", uint32 max = 0, uint32 current = 0, bool useSound = false);
    LocalNotificationText* const CreateNotificationText(const WideString& title = L"", const WideString& text = L"", bool useSound = false);
    void PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound = false);
    void RemoveAllDelayedNotifications();
    bool Remove(LocalNotification* const notification);
    bool RemoveById(const String& notificationId);
    void Clear();
    void Update();

    LocalNotification* const GetNotificationById(const String& id);
    void OnNotificationPressed(const String& id);

private:
    Mutex notificationsListMutex;
    List<LocalNotification*> notificationsList;
};
}

#endif // __NOTIFICATION_H__
