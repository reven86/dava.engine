#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include <jni.h>
#include <android/log.h>

namespace DAVA
{
class LocalNotificationController;

namespace Private
{
struct NativeDelegate final
{
    NativeDelegate(LocalNotificationController& controller);
    ~NativeDelegate();

private:
    jobject instance;
    Function<void(jobject)> release;
};
} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
