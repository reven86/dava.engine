#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__)

#include "Notification/Private/LocalNotificationImpl.h"

#include "Base/Message.h"

namespace DAVA
{
struct NSUserNotificationWrapper;

class LocalNotificationMac : public LocalNotificationImpl
{
public:
    LocalNotificationMac(const String& _id);
    ~LocalNotificationMac() override;

    void SetAction(const WideString& action) override;
    void Hide() override;
    void ShowText(const WideString& title, const WideString& text, bool useSound) override;
    void ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound) override;
    void PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound) override;
    void RemoveAllDelayedNotifications() override;

public:
    NSUserNotificationWrapper* notification;
};
}

#endif // defined(__DAVAENGINE_MACOS__)
