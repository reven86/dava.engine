/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __FRAMEWORK__DEVICEINFO__
#define __FRAMEWORK__DEVICEINFO__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

namespace DAVA
{

class DeviceInfo
{
public:
	enum ePlatform
	{
		PLATFORM_MACOS = 0,
		PLATFORM_IOS,
		PLATFORM_IOS_SIMULATOR,
		PLATFORM_ANDROID,
		PLATFORM_WIN32,
		PLATFORM_UNKNOWN,
		PLATFORMS_COUNT
	};

    enum eNetworkType
	{
		NETWORK_TYPE_NOT_CONNECTED = 0,
		NETWORK_TYPE_UNKNOWN,
		NETWORK_TYPE_CELLULAR,
		NETWORK_TYPE_WIFI,
		NETWORK_TYPE_WIMAX,
		NETWORK_TYPE_ETHERNET,
		NETWORK_TYPE_BLUETOOTH,
	};

    static const int32 SIGNAL_STRENGTH_UNKNOWN = -1;
    struct NetworkInfo
    {
        eNetworkType networkType;
        int32 signalStrength; //(0-no signal, 100 - max signal)
        
        NetworkInfo()
        {
            networkType = NETWORK_TYPE_UNKNOWN;
            signalStrength = SIGNAL_STRENGTH_UNKNOWN;
        }
    };

    struct StorageRecord
    {
        String name;
        int64 totalSpace;
        int64 freeSpace;

        StorageRecord()
        :   name("")
        ,   totalSpace(0)
        ,   freeSpace(0)
        {}
    };

	static ePlatform GetPlatform();
	static String GetPlatformString();
	static String GetVersion();
	static String GetManufacturer();
	static String GetModel();
	static String GetLocale();
	static String GetRegion();
	static String GetTimeZone();
    static String GetUDID();
    static WideString GetName();
    static int GetZBufferSize();
    static eGPUFamily GetGPUFamily();
    static NetworkInfo GetNetworkInfo();
    static List<StorageRecord> GetStorageList();
};

};
#endif /* defined(__FRAMEWORK__DEVICEINFO__) */
