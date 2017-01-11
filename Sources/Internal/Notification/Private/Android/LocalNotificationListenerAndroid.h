#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

#include "Functional/Function.h"
#include <jni.h>
#include <android/log.h>

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct LocalNotificationListener final
{
    LocalNotificationListener(LocalNotificationController& controller);
    ~LocalNotificationListener();

private:
    jobject instance = nullptr;
    Function<void(jobject)> release;
};
} // namespace Private
} // namespace DAVA

#endif // defined(__DAVAENGINE_ANDROID__)
#endif // defined(__DAVAENGINE_COREV2__)
