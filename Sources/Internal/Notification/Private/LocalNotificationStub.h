#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_LINUX__)

#include "Notification/Private/LocalNotificationImpl.h"

namespace DAVA
{
class LocalNotificationStub : public LocalNotificationImpl
{
public:
    LocalNotificationStub(const String& _id);
    ~LocalNotificationStub() override;

    void SetAction(const WideString& action) override;
    void Hide() override;
    void ShowText(const WideString& title, const WideString& text, bool useSound) override;
    void ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound) override;
    void PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound) override;
    void RemoveAllDelayedNotifications() override;
};

LocalNotificationStub::LocalNotificationStub(const String& _id)
{
    notificationId = _id;
}

LocalNotificationStub::~LocalNotificationStub()
{
}

void LocalNotificationStub::SetAction(const WideString& action)
{
}

void LocalNotificationStub::Hide()
{
}
void LocalNotificationStub::ShowText(const WideString& title, const WideString& text, bool useSound)
{
}
void LocalNotificationStub::ShowProgress(const WideString& title, const WideString& text, uint32 total, uint32 progress, bool useSound)
{
}

LocalNotificationImpl* LocalNotificationImpl::Create(const String& _id)
{
    return new LocalNotificationStub(_id);
}

void LocalNotificationImpl::RequestPermissions()
{
}

void LocalNotificationStub::PostDelayedNotification(const WideString& title, const WideString& text, int delaySeconds, bool useSound)
{
}

void LocalNotificationStub::RemoveAllDelayedNotifications()
{
}
} // namespace DAVA

#endif // defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_LINUX__)
