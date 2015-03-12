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


#include "DeviceInfoTest.h"
#include "Platform/DeviceInfo.h"
#include "Platform/DateTime.h"

DeviceInfoTest::DeviceInfoTest()
:	UITestTemplate<DeviceInfoTest>("DeviceInfoTest"),
    deviceInfoText(NULL)
{
	RegisterFunction(this, &DeviceInfoTest::TestFunction, Format("DeviceInfo test"), NULL);
}

void DeviceInfoTest::LoadResources()
{
    UITestTemplate::LoadResources();
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
	
    Rect textRect = GetRect();
    textRect.SetPosition(textRect.GetPosition() + Vector2(1.0f, 31.0f));
    textRect.SetSize(textRect.GetSize() - Vector2(1.0f, 31.0f));

	deviceInfoText = new UIStaticText(textRect);
    deviceInfoText->SetMultiline(true);
    deviceInfoText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    deviceInfoText->SetFont(font);
    deviceInfoText->SetTextColor(Color::White);
    deviceInfoText->SetDebugDraw(true);

    AddControl(deviceInfoText);
}

void DeviceInfoTest::UnloadResources()
{
    UITestTemplate::UnloadResources();
    SafeRelease(deviceInfoText);
}

void DeviceInfoTest::DidAppear()
{
    String platform = DeviceInfo::GetPlatformString();
    String version = DeviceInfo::GetVersion();
    String manufacturer = DeviceInfo::GetManufacturer();
    String model = DeviceInfo::GetModel();
    String locale = DeviceInfo::GetLocale();
    String region = DeviceInfo::GetRegion();
    String timezone = DeviceInfo::GetTimeZone();
    String udid = DeviceInfo::GetUDID();
    String proxyHost = DeviceInfo::GetHTTPProxyHost();
    int proxyPort = DeviceInfo::GetHTTPProxyPort();
    String proxyExculde = DeviceInfo::GetHTTPNonProxyHosts();
    WideString name = DeviceInfo::GetName();
    List<DeviceInfo::StorageInfo> storages = DeviceInfo::GetStoragesList();
    
    String deviceInfoString;
    deviceInfoString += Format("Platform: %s\n", platform.c_str());
    deviceInfoString += Format("OS version: %s\n", version.c_str());
	deviceInfoString += Format("Manufacturer: %s\n", manufacturer.c_str());
	deviceInfoString += Format("Model: %s\n", model.c_str());
	deviceInfoString += Format("Locale: %s\n", locale.c_str());
	deviceInfoString += Format("Region: %s\n", region.c_str());
	deviceInfoString += Format("Time zone: %s\n", timezone.c_str());
    deviceInfoString += Format("UDID: %s\n", udid.c_str());
    deviceInfoString += Format("Name: %s\n", WStringToString(name).c_str());
    deviceInfoString += Format("ZBufferSize: %d\n", DeviceInfo::GetZBufferSize());
    deviceInfoString += Format("Proxy Host: %s\n", proxyHost.c_str());
    deviceInfoString += Format("Proxy Port: %d\n", proxyPort);
    deviceInfoString += Format("Proxy Exclude Hosts: %s\n", proxyExculde.c_str());
    deviceInfoString += Format("CPU count: %d\n", DeviceInfo::GetCpuCount());
	const eGPUFamily gpu = DeviceInfo::GetGPUFamily();
	if(gpu == GPU_INVALID)
	{
		deviceInfoString += "GPU family: INVALID\n";
	}
	else
	{
		deviceInfoString += Format("GPU family: %s\n", GPUFamilyDescriptor::GetGPUName(gpu).c_str());
	}
    deviceInfoString += Format("Network connection type: %s\n", GetNetworkTypeString().c_str());
    deviceInfoString += Format("Network signal strength: %i%%\n", DeviceInfo::GetNetworkInfo().signalStrength);

    List<DeviceInfo::StorageInfo>::const_iterator iter = storages.begin();
    for (;iter != storages.end(); ++iter)
    {
    	String storageName;

    	switch(iter->type)
    	{
    		case DeviceInfo::STORAGE_TYPE_INTERNAL:
    			storageName = "Internal storage";
    			break;

    		case DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL:
    			storageName = "Primary external storage";
    			break;

    		case DeviceInfo::STORAGE_TYPE_SECONDARY_EXTERNAL:
    			storageName = "Secondary external storage";
    			break;

    		default:
    			storageName = "Unknown storage";
    			break;
    	}

    	String str;
    	if (iter->emulated)
    	{
    		str += "; emulated";
    	}
    	if (iter->readOnly)
    	{
    		str += "; read only";
    	}

    	deviceInfoString += Format("%s: path: %s; capacity: %s; free: %s%s\n", storageName.c_str(),
    			iter->path.GetAbsolutePathname().c_str(), FormatStorageSize(iter->totalSpace).c_str(),
    			FormatStorageSize(iter->freeSpace).c_str(), str.c_str());
    }

    deviceInfoText->SetText(StringToWString(deviceInfoString));
	Logger::Debug("********** Device info **********");
	Logger::Debug(deviceInfoString.c_str());
	Logger::Debug("********** Device info **********");
}

String DeviceInfoTest::FormatStorageSize(int64 size)
{
	const char prefix[] = {' ', 'K', 'M', 'G'};

	float32 res = (float)size;
	int32 i = 0;

	while (i < sizeof(prefix) - 1 && res >= 1024.f)
	{
		++i;
		res /= 1024.f;
	}

	return Format("%.2f %cB", res, prefix[i]);
}

void DeviceInfoTest::TestFunction(TestTemplate<DeviceInfoTest>::PerfFuncData *data)
{
	return;
}

String DeviceInfoTest::GetNetworkTypeString()
{
    static const struct
    {
        DeviceInfo::eNetworkType networkType;
        String networkTypeString;
    } networkTypesMap[] =
    {
        { DeviceInfo::NETWORK_TYPE_NOT_CONNECTED, "Not Connected" },
        { DeviceInfo::NETWORK_TYPE_CELLULAR, "Cellular" },
        { DeviceInfo::NETWORK_TYPE_WIFI, "Wi-Fi" },
        { DeviceInfo::NETWORK_TYPE_WIMAX, "WiMAX" },
        { DeviceInfo::NETWORK_TYPE_ETHERNET, "Ehternet" },
        { DeviceInfo::NETWORK_TYPE_BLUETOOTH, "Bluetooth" },
        { DeviceInfo::NETWORK_TYPE_UNKNOWN, "Unknown" }
    };
    
    const DeviceInfo::NetworkInfo& networkInfo = DeviceInfo::GetNetworkInfo();
    uint32 networkTypesCount = COUNT_OF(networkTypesMap);
    for (uint32 i = 0; i < networkTypesCount; i ++)
    {
        if (networkTypesMap[i].networkType == networkInfo.networkType)
        {
            return networkTypesMap[i].networkTypeString;
        }
    }
    
    return "Unknown";
}
