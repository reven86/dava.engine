#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct NativeDelegate
{
    NativeDelegate(LocalNotificationController& controller);
};
} // namespace Private
} // namespace DAVA

#endif //__DAVAENGINE_WIN32__
