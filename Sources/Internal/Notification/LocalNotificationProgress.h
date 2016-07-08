#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_PROGRESS_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_PROGRESS_H__

#include "Notification/LocalNotification.h"

namespace DAVA
{
class LocalNotificationProgress : public LocalNotification
{
    friend class LocalNotificationController;

protected:
    LocalNotificationProgress();
    ~LocalNotificationProgress() override;

public:
    void SetProgressCurrent(const uint32 _currentProgress);
    void SetProgressTotal(const uint32 _total);

private:
    void ImplShow() override;

private:
    uint32 total;
    uint32 progress;
};
}

#endif
