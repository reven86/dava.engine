#ifndef __FRAMEWORK__DEVICEINFO_PRIVATE_BASE__
#define __FRAMEWORK__DEVICEINFO_PRIVATE_BASE__

#include "Platform/DeviceInfo.h"

namespace DAVA
{
//Common implementation of device info
class DeviceInfoPrivateBase
{
public:
    int32 GetCpuCount();
    DeviceInfo::HIDConnectionSignal& GetHIDConnectionSignal(DeviceInfo::eHIDType type);
    // default implementation, could be changed in inheritors

private:
    Map<DeviceInfo::eHIDType, DeviceInfo::HIDConnectionSignal> hidConnectionSignals;
};

} // namespace DAVA

#endif // __FRAMEWORK__DEVICEINFO_PRIVATE_BASE__