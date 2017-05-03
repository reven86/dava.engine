#include "Base/BaseTypes.h"
#include "Platform/DPIHelper.h"
#include "Platform/DeviceInfo.h"
#include "Core/Core.h"

#include <UIKit/UIKit.h>
#include <UIKit/UIDevice.h>

#if !defined(__DAVAENGINE_COREV2__)

#define NO_DPI_INFO_FOUND 0

namespace DAVA
{
struct DeviceScreenInfo
{
    uint32 minimumSideLength;
    uint32 screenDPI;
    String deviceName;

    DeviceScreenInfo(uint32 _minimumSideLength, uint32 _screenDPI, String _deviceName)
    {
        minimumSideLength = _minimumSideLength;
        screenDPI = _screenDPI;
        deviceName = _deviceName;
    }
};

enum eIOS_DPI
{
    IPHONE_3_IPAD_MINI = 163,
    IPHONE_4_5_6_SE_IPAD_MINI2_MINI3 = 326,
    IPAD_1_2 = 132,
    IPAD_3_4_5_AIR_AIR2_PRO = 264,
    IPHONE_6_PLUS = 401,
    IPHONE_6_PLUS_ZOOM = 461,
};

const static DeviceScreenInfo devicesInfoList[] =
{
  DeviceScreenInfo(320, IPHONE_3_IPAD_MINI, ""),
  DeviceScreenInfo(640, IPHONE_4_5_6_SE_IPAD_MINI2_MINI3, ""),
  DeviceScreenInfo(750, IPHONE_4_5_6_SE_IPAD_MINI2_MINI3, ""),
  DeviceScreenInfo(768, IPAD_1_2, ""),
  DeviceScreenInfo(768, IPHONE_3_IPAD_MINI, "mini"),
  DeviceScreenInfo(1080, IPHONE_6_PLUS, ""),
  DeviceScreenInfo(1242, IPHONE_6_PLUS_ZOOM, ""),
  DeviceScreenInfo(1536, IPAD_3_4_5_AIR_AIR2_PRO, ""),
  DeviceScreenInfo(1536, IPHONE_4_5_6_SE_IPAD_MINI2_MINI3, "mini"),
  DeviceScreenInfo(2048, IPAD_3_4_5_AIR_AIR2_PRO, "")
};

uint32 GetDPIInfoListByDimension(uint32 minDimension, List<const DeviceScreenInfo*>& outputList)
{
    outputList.clear();

    size_t numDeviceInfos = sizeof(devicesInfoList) / sizeof(devicesInfoList[0]);
    for (size_t i = 0; i < numDeviceInfos; ++i)
    {
        if (devicesInfoList[i].minimumSideLength == minDimension)
        {
            outputList.push_back(&devicesInfoList[i]);
        }
    }

    return static_cast<uint32>(outputList.size());
}

uint32 DeterminateExactDPI(List<const DeviceScreenInfo*>& devList)
{
    String model = DeviceInfo::GetModel();
    NSString* deviceModel = [NSString stringWithCString:model.c_str()
                                               encoding:[NSString defaultCStringEncoding]];

    uint32 retDPI = NO_DPI_INFO_FOUND;

    for (List<const DeviceScreenInfo*>::const_iterator it = devList.begin(); it != devList.end(); ++it)
    {
        if ((*it)->deviceName == "")
        {
            if (NO_DPI_INFO_FOUND == retDPI)
            {
                retDPI = (*it)->screenDPI; // set default value
            }
        }
        else
        {
            NSString* searchString = [NSString stringWithCString:(*it)->deviceName.c_str()
                                                        encoding:[NSString defaultCStringEncoding]];

            NSRange range = [deviceModel rangeOfString:searchString options:NSCaseInsensitiveSearch];
            if (range.location != NSNotFound)
            {
                retDPI = (*it)->screenDPI;
                break;
            }
        }
    }
    return retDPI;
}

uint32 DPIHelper::GetScreenDPI()
{
    //due to magnificent api of ios the only way of determination of dpi is hardcode
    CGRect screenRect = [[UIScreen mainScreen] bounds];

    CGFloat scale = Core::Instance()->GetScreenScaleFactor();
    CGFloat screenWidth = screenRect.size.width * scale;
    CGFloat screenHeight = screenRect.size.height * scale;

    NSLog(@"w=%f h=%f", screenWidth, screenHeight);

    //width and height could be swapped according orientation
    uint32 minRes = screenWidth > screenHeight ? screenHeight : screenWidth;

    List<const DeviceScreenInfo*> outputList;
    uint32 foundedDevices = GetDPIInfoListByDimension(minRes, outputList);

    if (foundedDevices == 0)
    {
        return NO_DPI_INFO_FOUND;
    }

    if (foundedDevices == 1)
    {
        return outputList.front()->screenDPI;
    }

    if (foundedDevices > 1)
    {
        return DeterminateExactDPI(outputList);
    }
    return NO_DPI_INFO_FOUND;
}
}

#endif
