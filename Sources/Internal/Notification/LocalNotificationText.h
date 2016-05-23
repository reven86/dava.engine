#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_TEXT_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_TEXT_H__

#include "Notification/LocalNotification.h"

namespace DAVA
{
class LocalNotificationText : public LocalNotification
{
private:
    void ImplShow() override;
};
}

#endif // __NOTIFICATION_H__
