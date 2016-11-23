#include "Notification/Private/NativeDelegateNotImplemented.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)

namespace DAVA
{
namespace Private
{
NativeDelegate::NativeDelegate(LocalNotificationController& controller)
{
}
} // namespace Private
} // namespace DAVA
#endif
