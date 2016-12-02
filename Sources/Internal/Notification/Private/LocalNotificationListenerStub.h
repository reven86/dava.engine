#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct LocalNotificationListener
{
    LocalNotificationListener(LocalNotificationController& controller);
};

LocalNotificationListener::LocalNotificationListener(LocalNotificationController& controller)
{
}
} // namespace Private
} // namespace DAVA

#endif // defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)
#endif // defined(__DAVAENGINE_COREV2__)
