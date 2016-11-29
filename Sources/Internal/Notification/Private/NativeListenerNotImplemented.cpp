#include "Notification/Private/NativeListenerNotImplemented.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)

namespace DAVA
{
namespace Private
{
NativeListener::NativeListener(LocalNotificationController& controller)
{
}
} // namespace Private
} // namespace DAVA
#endif // defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)
#endif // defined(__DAVAENGINE_COREV2__)
