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
:	TestTemplate<DeviceInfoTest>("DeviceInfoTest")
{
	RegisterFunction(this, &DeviceInfoTest::TestFunction, Format("DeviceInfo test"), NULL);
}

void DeviceInfoTest::LoadResources()
{
}

void DeviceInfoTest::UnloadResources()
{
}

void DeviceInfoTest::Draw(const DAVA::UIGeometricData &geometricData)
{
}

void DeviceInfoTest::TestFunction(TestTemplate<DeviceInfoTest>::PerfFuncData *data)
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

	Logger::Debug("********** Device info **********");
	Logger::Debug("Platform: %s", platform.c_str());
	Logger::Debug("OS version: %s", version.c_str());
	Logger::Debug("Manufacturer: %s", manufacturer.c_str());
	Logger::Debug("Model: %s", model.c_str());
	Logger::Debug("Locale: %s", locale.c_str());
	Logger::Debug("Region: %s", region.c_str());
	Logger::Debug("Time zone: %s", timezone.c_str());
    Logger::Debug("UDID: %s", udid.c_str());
    Logger::Debug("Name: %s", WStringToString(name).c_str());
    Logger::Debug("ZBufferSize: %d", DeviceInfo::GetZBufferSize());
	Logger::Debug("Proxy Host: %s", proxyHost.c_str());
	Logger::Debug("Proxy Port: %d", proxyPort);
	Logger::Debug("Proxy Exclude Hosts: %s", proxyExculde.c_str());
    Logger::Debug("GPU family: %s", GetGpuFamilyString(DeviceInfo::GetGPUFamily()).c_str());
	Logger::Debug("********** Device info **********");

	data->testData.message = "DeviceInfo test - passed";
	TEST_VERIFY(true);
}

String DeviceInfoTest::GetGpuFamilyString(eGPUFamily gpuFamily)
{
    String res;
    switch (gpuFamily)
    {
        case DAVA::GPU_ADRENO:
            res = "Adreno";
            break;

        case DAVA::GPU_MALI:
            res = "Mali";
            break;

        case DAVA::GPU_POWERVR_ANDROID:
            res = "PowerVR Android";
            break;

        case DAVA::GPU_POWERVR_IOS:
            res = "PowerVR iOS";
            break;

        case DAVA::GPU_TEGRA:
            res = "Tegra";
            break;

        case DAVA::GPU_UNKNOWN:
        default:
            res = "Unknown GPU family";
            break;
    }

    return res;
}
