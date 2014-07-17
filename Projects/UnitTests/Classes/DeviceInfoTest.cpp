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
#include "Platform/DateTime.h"

float32 DeviceInfoTest::AUTO_CLOSE_TIME = 30.f;

DeviceInfoTest::DeviceInfoTest()
:	TestTemplate<DeviceInfoTest>("DeviceInfoTest"),
    deviceInfoText(NULL),
    finishTestBtn(NULL),
    testFinished(false),
    onScreenTime(0.0f)
{
	RegisterFunction(this, &DeviceInfoTest::TestFunction, Format("DeviceInfo test"), NULL);
}

void DeviceInfoTest::LoadResources()
{
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
    
    finishTestBtn = new UIButton(Rect(1, 1, 100, 29));
	finishTestBtn->SetStateFont(0xFF, font);
    finishTestBtn->SetStateFontColor(0xFF, Color::White);
	finishTestBtn->SetStateText(0xFF, L"Finish test");
	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DeviceInfoTest::ButtonPressed));
	AddControl(finishTestBtn);
}

void DeviceInfoTest::UnloadResources()
{
    SafeRelease(deviceInfoText);
    SafeRelease(finishTestBtn);
}

void DeviceInfoTest::DidAppear()
{
    onScreenTime = 0.0f;
    
    String platform = DeviceInfo::GetPlatformString();
    String version = DeviceInfo::GetVersion();
    String manufacturer = DeviceInfo::GetManufacturer();
    String model = DeviceInfo::GetModel();
    String locale = DeviceInfo::GetLocale();
    String region = DeviceInfo::GetRegion();
    String timezone = DeviceInfo::GetTimeZone();
    String udid = DeviceInfo::GetUDID();
    WideString name = DeviceInfo::GetName();
    
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
    deviceInfoString += Format("Network signal strength: %i%%", DeviceInfo::GetNetworkInfo().signalStrength);

    deviceInfoText->SetText(StringToWString(deviceInfoString));
	Logger::Debug("********** Device info **********");
	Logger::Debug(deviceInfoString.c_str());
	Logger::Debug("********** Device info **********");
}

void DeviceInfoTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<DeviceInfoTest>::Update(timeElapsed);
}

bool DeviceInfoTest::RunTest(int32 testNum)
{
	TestTemplate<DeviceInfoTest>::RunTest(testNum);
	return testFinished;
}

void DeviceInfoTest::TestFunction(TestTemplate<DeviceInfoTest>::PerfFuncData *data)
{
	return;
}

void DeviceInfoTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    if (obj == finishTestBtn)
	{
		testFinished = true;
	}
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
