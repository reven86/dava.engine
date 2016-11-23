#include "Notification/Private/NativeDelegateNotImplemented.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)

namespace DAVA
{
NativeDelegate::NativeDelegate(LocalNotificationController& controller)
{
}
}
#endif
