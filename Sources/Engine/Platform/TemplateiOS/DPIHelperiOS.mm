/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/BaseTypes.h"
#include "Platform/DPIHelper.h"
#include "DeviceInfo.h"

#include <UIKit/UIKit.h>
#include <UIKit/UIDevice.h>

#define NO_DPI_INFO_FOUND   0
#define INFO_LIST_SIZE      5

namespace DAVA
{
    struct DeviceScreenInfo
    {
        uint32      minimumSideLength;
        uint32      screenDPI;
        String      deviceName;
        
        DeviceScreenInfo(uint32 _minimumSideLength, uint32 _screenDPI, String _deviceName)
        {
            minimumSideLength = _minimumSideLength;
            screenDPI         = _screenDPI;
            deviceName        = _deviceName;
        }
        
    };
    
    enum eIOS_DPI
    {
        IPHONE_3_IPAD_MINI      = 163,
        IPHONE_4_5              = 326,
        IPAD_1_2                = 132,
        IPAD_3_4                = 264
    };
    
        
    const static DeviceScreenInfo devicesInfoList[INFO_LIST_SIZE] =
    {
        DeviceScreenInfo(320, IPHONE_3_IPAD_MINI,  ""),
        DeviceScreenInfo(640, IPHONE_4_5,""),
        DeviceScreenInfo(768, IPAD_1_2,  ""),
        DeviceScreenInfo(768, IPHONE_3_IPAD_MINI, "mini"),
        DeviceScreenInfo(1536, IPAD_3_4, ""),

    };
    
    uint32 GetDPIInfoListByDimension(uint32 minDimension,  List<const DeviceScreenInfo*> & outputList)
    {
        outputList.clear();
        
        for (uint32 i = 0; i < INFO_LIST_SIZE; ++i)
        {
            if(devicesInfoList[i].minimumSideLength == minDimension)
            {
                outputList.push_back( &devicesInfoList[i]);
            }
        }
        
        return outputList.size();
    }
    
    uint32 DeterminateExactDPI(List<const DeviceScreenInfo*> &devList)
    {
		String model = DeviceInfo::GetModel();
		NSString* deviceModel = [NSString stringWithCString:model.c_str()
												   encoding:[NSString defaultCStringEncoding]];

        uint32 retDPI = NO_DPI_INFO_FOUND;
        
        for (List<const DeviceScreenInfo*>::const_iterator it = devList.begin(); it != devList.end(); ++it)
        {
            if((*it)->deviceName == "" )
            {
                if(NO_DPI_INFO_FOUND == retDPI)
                {
                    retDPI = (*it)->screenDPI;// set default value
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
        CGFloat scale = [[UIScreen mainScreen] scale]; //support of retina
        CGFloat screenWidth = screenRect.size.width * scale;
        CGFloat screenHeight = screenRect.size.height * scale;
        
         //width and height could be swapped according orientation
        uint32 minRes = screenWidth > screenHeight ? screenHeight : screenWidth;

        List<const DeviceScreenInfo*> outputList;
        uint32 foundedDevices = GetDPIInfoListByDimension(minRes, outputList);
        
        if (foundedDevices == 0)
        {
            return NO_DPI_INFO_FOUND;
        }

        if(foundedDevices == 1)
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


