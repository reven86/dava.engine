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

#include <algorithm>

#include <Base/FunctionTraits.h>
#include <Platform/DeviceInfo.h>
#include <Utils/UTF8Utils.h>
#include <Utils/StringFormat.h>

#include <Network/Base/IPAddress.h>
#include <Network/Base/Endpoint.h>

#include <Network/NetConfig.h>
#include <Network/NetCore.h>

#include "NetworkTest.h"

using namespace DAVA;
using namespace DAVA::Net;

NetworkTest::NetworkTest() : TestTemplate<NetworkTest>("NetworkTest")
                           , quitFlag(false)
                           , periodicFlag(false)
                           , loggerInUse(false)
                           , logger(true)
                           , btnDebug(NULL)
                           , btnInfo(NULL)
                           , btnWarn(NULL)
                           , btnError(NULL)
                           , btnPacket(NULL)
                           , btnPeriodic(NULL)
                           , btnQuit(NULL)
{
    new NetCore();

    NetCore::Instance()->RegisterService(SERVICE_LOG, MakeFunction(this, &NetworkTest::CreateLogger), MakeFunction(this, &NetworkTest::DeleteLogger));

    RegisterFunction(this, &NetworkTest::DummyTest, "DummyTest", NULL);

    Logger::Debug("NetworkTest ctor");
}

NetworkTest::~NetworkTest()
{
    // Do not anything as object is not deleted by test infrastructure
}

void NetworkTest::DummyTest(PerfFuncData* data)
{

}

void NetworkTest::LoadResources()
{
    CreateUI();
    {
        NetConfig config(SERVER_ROLE);
        config.AddTransport(TRANSPORT_TCP, Endpoint(9999));
        config.AddService(SERVICE_LOG);

        peerDescr = PeerDescription(config);

        Endpoint annoEndpoint("239.192.100.1", 9999);
        NetCore::Instance()->CreateAnnouncer(annoEndpoint, 5, MakeFunction(this, &NetworkTest::AnnounceDataSupplier));
        NetCore::Instance()->CreateController(config, NULL);
    }
    {
    	// Cannot log wide string uniformly on all platforms, maybe due to incorrect format flag
        Logger::Debug( "Name        : %s", UTF8Utils::EncodeToUTF8(DeviceInfo::GetName()).c_str());
        Logger::Debug( "Platfrom    : %s", DeviceInfo::GetPlatformString().c_str());
        Logger::Debug( "Model       : %s", DeviceInfo::GetModel().c_str());
        Logger::Debug( "Version     : %s", DeviceInfo::GetVersion().c_str());
        Logger::Debug( "Manufacturer: %s", DeviceInfo::GetManufacturer().c_str());
        Logger::Debug( "CPU count   : %d", DeviceInfo::GetCpuCount());
        Logger::Debug( "UDID        : %s", DeviceInfo::GetUDID().c_str());
    }
}

void NetworkTest::UnloadResources()
{
    logger.Uninstall();     // Uninstall logger here
    NetCore::Instance()->Finish(true);
    NetCore::Instance()->Release();
    DestroyUI();
}

bool NetworkTest::RunTest(int32 testNum)
{
    return !quitFlag ? false
                     : TestTemplate<NetworkTest>::RunTest(testNum);
}

void NetworkTest::Update(float32 timeElapsed)
{
    static uint32 frameCount = 0;
    static float32 timeCounter = 0.0f;
    if (periodicFlag)
    {
        frameCount += 1;
        timeCounter += timeElapsed;
        if (timeCounter >= 1.0f)
        {
            Logger::Info("Periodic: timeCounter=%.1f, frameCount=%u", timeCounter, frameCount);
            frameCount = 0;
            timeCounter = 0.0f;
        }
    }
    else
    {
        frameCount = 0;
        timeCounter = 0.0f;
    }
    TestTemplate<NetworkTest>::Update(timeElapsed);
    NetCore::Instance()->Poll();
}

IChannelListener* NetworkTest::CreateLogger(uint32 serviceId, void* context)
{
    if (false == loggerInUse)
    {
        loggerInUse = true;
        return &logger;
    }
    return NULL;
}

void NetworkTest::DeleteLogger(IChannelListener* obj, void* context)
{
    loggerInUse = false;
}

size_t NetworkTest::AnnounceDataSupplier(size_t length, void* buffer)
{
    size_t sz = peerDescrSerialized.size();
    if (peerDescrSerialized.empty())
    {
        sz = peerDescr.SerializedSize();
        peerDescrSerialized.resize(sz);
        void* p = &*peerDescrSerialized.begin();
        if (0 == peerDescr.Serialize(p, sz))
            peerDescrSerialized.clear();
    }
    if (!peerDescrSerialized.empty() && peerDescrSerialized.size() <= length)
    {
        Memcpy(buffer, &*peerDescrSerialized.begin(), peerDescrSerialized.size());
        return peerDescrSerialized.size();
    }
    return 0;
}

void NetworkTest::CreateUI()
{
    static const float32 BUTTON_WIDTH = 300.0f;
    static const float32 BUTTON_HEIGHT = 80.0f;
    static const float32 HORZ_SPACE = 100.0f;
    static const float32 VERT_SPACE = 10.0f;

    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(20);

    float32 x = 10.0f;
    float32 y = 10.0f;
    btnDebug = CreateButton(L"Log debug", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);
    y += BUTTON_HEIGHT + VERT_SPACE;
    btnInfo = CreateButton(L"Log info", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);
    y += BUTTON_HEIGHT + VERT_SPACE;
    btnWarn = CreateButton(L"Log warn", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);
    y += BUTTON_HEIGHT + VERT_SPACE;
    btnError = CreateButton(L"Log error", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);
    y += BUTTON_HEIGHT * 2.0f;
    btnQuit = CreateButton(L"Quit", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);

    x = 10.0f + BUTTON_WIDTH + HORZ_SPACE;
    y = 10.0f;
    btnPacket = CreateButton(L"Log packet", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);
    y += BUTTON_HEIGHT + VERT_SPACE;
    btnPeriodic = CreateButton(L"Periodic off", Rect(x, y, BUTTON_WIDTH, BUTTON_HEIGHT), font);
    SafeRelease(font);
}

UIButton* NetworkTest::CreateButton(const wchar_t* caption, const DAVA::Rect& rc, DAVA::Font* font)
{
    UIButton* button = new UIButton;
    button->SetStateFont(0xFF, font);
    button->SetDebugDraw(true);
    button->SetStateText(0xFF, caption);
    button->SetRect(rc);
    button->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NetworkTest::ButtonPressed));
    AddControl(button);
    return button;
}

void NetworkTest::DestroyUI()
{
    SafeRelease(btnDebug);
    SafeRelease(btnInfo);
    SafeRelease(btnWarn);
    SafeRelease(btnError);
    SafeRelease(btnPacket);
    SafeRelease(btnPeriodic);
    SafeRelease(btnQuit);
}

void NetworkTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    if (obj == btnDebug)
        Logger::Debug("This is DEBUG message");
    else if (obj == btnInfo)
        Logger::Info("This is INFO message");
    else if (obj == btnWarn)
        Logger::Warning("This is WARN message");
    else if (obj == btnError)
        Logger::Error("This is ERROR message");
    else if (obj == btnPacket)
    {
        Logger::Debug("#1. DEBUG");
        Logger::Info("#2. INFO");
        Logger::Warning("#3. WARN");
        Logger::Error("#4. ERROR");
    }
    else if (obj == btnPeriodic)
    {
        periodicFlag = !periodicFlag;
        if (periodicFlag)
        {
            btnPeriodic->SetStateText(0xFF, L"Periodic on");
            Logger::Info("Periodic on");
        }
        else
        {
            btnPeriodic->SetStateText(0xFF, L"Periodic off");
            Logger::Info("Periodic off");
        }
    }
    else if (obj == btnQuit)
    {
        quitFlag = true;
        Logger::Info("Quiting..");
    }
}
