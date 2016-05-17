#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_UAP_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_UAP_H__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Notification/LocalNotificationImpl.h"
#include "Base/Message.h"

namespace DAVA
{
class LocalNotificationUAP : public LocalNotificationImpl
{
public:
    LocalNotificationUAP(const String& _id);

    void SetAction(const WideString& action) override;
    void Hide() override;
    void ShowText(const WideString& title, const WideString& text, bool useSound) override;
    void ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound) override;
    void PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound) override;
    void RemoveAllDelayedNotifications();

private:
    void CreateOrUpdateNotification(Windows::Data::Xml::Dom::XmlDocument ^ notificationDeclaration,
                                    const Windows::Foundation::DateTime* startTime = nullptr,
                                    bool ghostNotification = false);

    Windows::UI::Notifications::ToastNotifier ^ toastNotifier;
    Windows::UI::Notifications::ToastNotification ^ notification;
};
}

#endif

#endif /* defined __DAVAENGINE_LOCAL_NOTIFICATION_UAP_H__ */
