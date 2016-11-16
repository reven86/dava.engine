#include "Notification/Private/LocalNotificationNotImplemented.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)

namespace DAVA
{
NativeDelegateImpl::NativeDelegateImpl(LocalNotificationController& controller)
{
}
}
#endif
