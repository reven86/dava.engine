#pragma once

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Message.h"
#include "Concurrency/Mutex.h"

namespace DAVA
{
class LocalNotification;
class LocalNotificationText;
class LocalNotificationProgress;

#if defined(__DAVAENGINE_COREV2__)
namespace Private
{
struct LocalNotificationListener;
}
#endif

class LocalNotificationController : public Singleton<LocalNotificationController>
{
    friend class LocalNotification;

public:
    LocalNotificationController();
    virtual ~LocalNotificationController();
    LocalNotificationProgress* const CreateNotificationProgress(const WideString& title = L"", const WideString& text = L"", uint32 max = 0, uint32 current = 0, bool useSound = false);
    LocalNotificationText* const CreateNotificationText(const WideString& title = L"", const WideString& text = L"", bool useSound = false);
    void PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound = false);
    void RemoveAllDelayedNotifications();
    bool Remove(LocalNotification* const notification);
    bool RemoveById(const String& notificationId);
    void Clear();
    void Update();

    void RequestPermissions();

    LocalNotification* const GetNotificationById(const String& id);
    void OnNotificationPressed(const String& id);

private:
    Mutex notificationsListMutex;
    List<LocalNotification*> notificationsList;
#if defined(__DAVAENGINE_COREV2__)
    std::unique_ptr<Private::LocalNotificationListener> localListener;
#endif
};
} // namespace DAVA
