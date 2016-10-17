#pragma once
#include "Platform/DeviceInfo.h"

namespace DAVA
{
//Common implementation of device info
class DeviceInfoPrivateBase
{
public:
    int32 GetCpuCount();
    DeviceInfo::HIDConnectionSignal& GetHIDConnectionSignal(DeviceInfo::eHIDType type);

    eGPUFamily GetGPUFamily();
    virtual eGPUFamily GetGPUFamilyImpl() = 0;

    void SetOverrideGPU(eGPUFamily newGPU);
    void ResetOverrideGPU();

private:
    Map<DeviceInfo::eHIDType, DeviceInfo::HIDConnectionSignal> hidConnectionSignals;
    eGPUFamily overridenGPU = GPU_INVALID;
};

} // namespace DAVA
