#include "Notification/Private/NativeDelegate.h"
#include "Notification/LocalNotificationController.h"

namespace DAVA
{
NativeDelegate::NativeDelegate(LocalNotificationController& controller)
    : NativeDelegateImpl(controller)
{
}
}