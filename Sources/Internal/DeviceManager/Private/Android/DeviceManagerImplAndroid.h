#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_ANDROID__)

#include "DeviceManager/DeviceManagerTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Android/JNIBridge.h"

namespace DAVA
{
class DeviceManager;
namespace Private
{
struct DeviceManagerImpl final
{
    DeviceManagerImpl(DeviceManager* devManager, Private::MainDispatcher* dispatcher);

    void UpdateDisplayConfig();

    DisplayInfo ConvertFromJavaDisplayInfo(JNIEnv* env, const jobject javaDisplayInfo, const bool isPrimary);

    float32 GetCpuTemperature() const;

    DeviceManager* deviceManager = nullptr;
    Private::MainDispatcher* mainDispatcher = nullptr;

    // JNI part
    JNI::JavaClass javaDeviceManagerClass;
    JNI::JavaClass javaDisplayInfoClass;
    JNI::GlobalRef<jobject> javaDeviceManagerInstance;
    Function<jfloat(jobject)> javaGetCpuTemperatureMethod;
    jfieldID javaDisplayInfoNameField;
    jfieldID javaDisplayInfoIdField;
    jfieldID javaDisplayInfoWidthField;
    jfieldID javaDisplayInfoHeightField;
    jfieldID javaDisplayInfoDpiXField;
    jfieldID javaDisplayInfoDpiYField;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_COREV2__
