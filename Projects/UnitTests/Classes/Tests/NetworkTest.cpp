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

TestEchoServer::TestEchoServer()
    : testDone(false)
    , bytesRecieved(0)
    , bytesSent(0)
    , bytesDelivered(0)
    , lastPacketId(0)
{

}

void TestEchoServer::OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length)
{
    bytesRecieved += length;
    SendEcho(buffer, length);
}

void TestEchoServer::OnPacketSent(IChannel* aChannel, const void* buffer, size_t length)
{
    // buffer must be among sent buffers and its length must correspond to sent length
    Deque<Parcel>::iterator i = std::find(parcels.begin(), parcels.end(), buffer);
    if (i != parcels.end())
    {
        Parcel& parcel = *i;
        if (parcel.length == length)
        {
            // Check whether we have echoed end marker
            if (3 == length && 0 == strncmp(static_cast<const char8*>(buffer), "END", 3))
            {
                lastPacketId = parcel.packetId; // Save packet ID for end marker
            }
            Free(parcel.outbuf);
            parcel.outbuf = NULL;
        }
        bytesSent += length;
    }
}

void TestEchoServer::OnPacketDelivered(IChannel* aChannel, uint32 packetId)
{
    if (false == parcels.empty())
    {
        // Delivery notifications must arrive in order of send operations
        Parcel parcel = parcels.front();
        parcels.pop_front();

        if (parcel.packetId == packetId)
        {
            bytesDelivered += parcel.length;
        }
    }
    if (packetId == lastPacketId)
    {
        // End marker has been recieved, echoed and confirmed, so testing is done
        testDone = true;
    }
}

void TestEchoServer::SendEcho(const void* buffer, size_t length)
{
    parcels.push_back(Parcel());
    Parcel& parcel = parcels.back();

    parcel.outbuf = Alloc(length);
    parcel.length = length;
    parcel.packetId = 0;
    Memcpy(parcel.outbuf, buffer, length);

    Send(parcel.outbuf, parcel.length, &parcel.packetId);
}

//////////////////////////////////////////////////////////////////////////
TestEchoClient::TestEchoClient()
    : testDone(false)
    , bytesRecieved(0)
    , bytesSent(0)
    , bytesDelivered(0)
    , pendingRead(0)
    , pendingSent(0)
    , pendingDelivered(0)
{
    // Prepare data of various length
    Parcel a[] = {
        {Alloc(1)       , 1       , 0},
        {Alloc(1000)    , 1000    , 0},
        {Alloc(10000)   , 10000   , 0},
        {Alloc(100000)  , 100000  , 0},
        {Alloc(1000000) , 1000000 , 0},
        {Alloc(10000000), 10000000, 0}
    };
    uint8 v = 'A';
    for (size_t i = 0;i < COUNT_OF(a);++i, ++v)
    {
        Memset(a[i].outbuf, v, a[i].length);
        parcels.push_back(a[i]);
    }

    // Prepare end marker
    Parcel end = {Alloc(3), 3, 0};
    Memcpy(end.outbuf, "END", 3);
    parcels.push_back(end);
}

TestEchoClient::~TestEchoClient()
{
    for (size_t i = 0, n = parcels.size();i < n;++i)
        Free(parcels[i].outbuf);
}

void TestEchoClient::ChannelOpen()
{
    // Send all parcell at a time
    for (size_t i = 0, n = parcels.size();i < n;++i)
        SendParcel(&parcels[i]);
}

void TestEchoClient::OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length)
{
    if (pendingRead < parcels.size())
    {
        Parcel& p = parcels[pendingRead];
        if (p.length == length && 0 == Memcmp(p.outbuf, buffer, length))
        {
            bytesRecieved += length;
        }
        pendingRead += 1;
    }

    // Check for end marker echoed from server
    if (3 == length && 0 == strncmp(static_cast<const char8*>(buffer), "END", 3))
    {
        testDone = true;
    }
}

void TestEchoClient::OnPacketSent(IChannel* aChannel, const void* buffer, size_t length)
{
    if (pendingSent < parcels.size())
    {
        // Check that send operation is sequential
        Parcel& p = parcels[pendingSent];
        if (p.length == length && 0 == Memcmp(p.outbuf, buffer, length))
        {
            bytesSent += length;
        }
        pendingSent += 1;
    }
}

void TestEchoClient::OnPacketDelivered(IChannel* aChannel, uint32 packetId)
{
    if (pendingDelivered < parcels.size())
    {
        // Check that delivery notification is sequential
        Parcel& p = parcels[pendingDelivered];
        if (p.packetId == packetId)
        {
            bytesDelivered += p.length;
        }
        pendingDelivered += 1;
    }
}

void TestEchoClient::SendParcel(Parcel* parcel)
{
    Send(parcel->outbuf, parcel->length, &parcel->packetId);
}

//////////////////////////////////////////////////////////////////////////
NetworkTest::NetworkTest() : TestTemplate<NetworkTest>("NetworkTest")
                           , testingEcho(true)
                           , logger()
                           , serverBytesRecv(NULL)
                           , serverBytesSent(NULL)
                           , serverBytesDelivered(NULL)
                           , clientBytesRecv(NULL)
                           , clientBytesSent(NULL)
                           , clientBytesDelivered(NULL)
                           , timeLeft(NULL)
{
    new NetCore();

    NetCore::Instance()->RegisterService(SERVICE_LOG, MakeFunction(this, &NetworkTest::CreateLogger), MakeFunction(this, &NetworkTest::DeleteLogger));
    NetCore::Instance()->RegisterService(SERVICE_ECHO, MakeFunction(this, &NetworkTest::CreateEcho), MakeFunction(this, &NetworkTest::DeleteEcho));

    RegisterFunction(this, &NetworkTest::TestEcho, "TestEcho", NULL);
    RegisterFunction(this, &NetworkTest::TestIPAddress, "TestIPAddress", NULL);
    RegisterFunction(this, &NetworkTest::TestEndpoint, "TestEndpoint", NULL);
    RegisterFunction(this, &NetworkTest::TestNetConfig, "TestNetConfig", NULL);
}

NetworkTest::~NetworkTest()
{
    // Do not anything as object is not deleted by test infrastructure
}

void NetworkTest::LoadResources()
{
    CreateUI();
    {
        NetConfig loggerConfig(SERVER_ROLE);
        loggerConfig.AddTransport(TRANSPORT_TCP, Endpoint(LOGGER_PORT));
        loggerConfig.AddService(SERVICE_LOG);

        NetCore::Instance()->CreateController(loggerConfig);
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

    NetConfig serverConfig(SERVER_ROLE);
    serverConfig.AddTransport(TRANSPORT_TCP, Endpoint(ECHO_PORT));
    serverConfig.AddService(SERVICE_ECHO);

    NetConfig clientConfig = serverConfig.Mirror(IPAddress("127.0.0.1"));

    // Server config must be recreated first due to restrictions of service management
    NetCore::Instance()->CreateController(serverConfig, reinterpret_cast<void*>(ECHO_SERVER_CONTEXT));
    NetCore::Instance()->CreateController(clientConfig, reinterpret_cast<void*>(ECHO_CLIENT_CONTEXT));
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
    return testingEcho ? false
                       : TestTemplate<NetworkTest>::RunTest(testNum);
}

void NetworkTest::Update(float32 timeElapsed)
{
    if (echoServer.IsTestDone() && echoClient.IsTestDone())
    {
        static float32 delay = 0.0f;
        delay += timeElapsed;
        if (delay > 10.0f)  // wait 10 sec after finishing test to allow logger to send enqueued records
            testingEcho = false;
        UpdateUI(true, 10.0f - delay);
    }
    else
        UpdateUI(false, 0.0f);
    NetCore::Instance()->Poll();

    TestTemplate<NetworkTest>::Update(timeElapsed);
}

void NetworkTest::TestEcho(PerfFuncData* data)
{
    TEST_VERIFY(echoServer.BytesRecieved() == echoServer.BytesSent());
    TEST_VERIFY(echoServer.BytesRecieved() == echoServer.BytesDelivered());

    TEST_VERIFY(echoClient.BytesRecieved() == echoClient.BytesSent());
    TEST_VERIFY(echoClient.BytesRecieved() == echoClient.BytesDelivered());

    TEST_VERIFY(echoServer.BytesRecieved() == echoClient.BytesRecieved());
}

void NetworkTest::TestIPAddress(PerfFuncData* data)
{
    // Test empty address
    TEST_VERIFY(true == IPAddress().IsUnspecified());
    TEST_VERIFY(0 == IPAddress().ToUInt());
    TEST_VERIFY("0.0.0.0" == IPAddress().ToString());

    // Test invalid address
    TEST_VERIFY(true == IPAddress("").IsUnspecified());
    TEST_VERIFY(true == IPAddress("invalid").IsUnspecified());
    TEST_VERIFY(true == IPAddress("300.0.1.2").IsUnspecified());
    TEST_VERIFY(true == IPAddress("08.08.0.1").IsUnspecified());

    // Test multicast address
    TEST_VERIFY(false == IPAddress("239.192.100.1").IsUnspecified());
    TEST_VERIFY(true == IPAddress("239.192.100.1").IsMulticast());
    TEST_VERIFY(false == IPAddress("192.168.0.4").IsMulticast());
    TEST_VERIFY(false == IPAddress("255.255.255.255").IsMulticast());
    TEST_VERIFY("239.192.100.1" == IPAddress("239.192.100.1").ToString());

    // Test address
    TEST_VERIFY(IPAddress("192.168.0.4") == IPAddress("192.168.0.4"));
    TEST_VERIFY(String("192.168.0.4") == IPAddress("192.168.0.4").ToString());
    TEST_VERIFY(IPAddress("192.168.0.4").ToString() == IPAddress::FromString("192.168.0.4").ToString());
    TEST_VERIFY(false == IPAddress("192.168.0.4").IsUnspecified()); 
    TEST_VERIFY(false == IPAddress("255.255.255.255").IsUnspecified());
}

void NetworkTest::TestEndpoint(PerfFuncData* data)
{
    TEST_VERIFY(0 == Endpoint().Port());
    TEST_VERIFY(String("0.0.0.0:0") == Endpoint().ToString());

    TEST_VERIFY(1234 == Endpoint("192.168.1.45", 1234).Port());
    TEST_VERIFY(Endpoint("192.168.1.45", 1234).Address() == IPAddress("192.168.1.45"));
    TEST_VERIFY(Endpoint("192.168.1.45", 1234).Address() == IPAddress::FromString("192.168.1.45"));

    TEST_VERIFY(Endpoint("192.168.1.45", 1234) == Endpoint("192.168.1.45", 1234));
    TEST_VERIFY(false == (Endpoint("192.168.1.45", 1234) == Endpoint("192.168.1.45", 1235)));  // Different ports
    TEST_VERIFY(false == (Endpoint("192.168.1.45", 1234) == Endpoint("192.168.1.46", 1234)));  // Different addressess
}

void NetworkTest::TestNetConfig(PerfFuncData* data)
{
    TEST_VERIFY(SERVER_ROLE == NetConfig(SERVER_ROLE).Role());
    TEST_VERIFY(false == NetConfig(SERVER_ROLE).Validate());

    NetConfig config1(SERVER_ROLE);
    config1.AddTransport(TRANSPORT_TCP, Endpoint(9999));
    config1.AddTransport(TRANSPORT_TCP, Endpoint(8888));
    TEST_VERIFY(false == config1.Validate());
    TEST_VERIFY(2 == config1.Transports().size());
    config1.AddService(3);
    config1.AddService(1);
    config1.AddService(2);
    TEST_VERIFY(true == config1.Validate());
    TEST_VERIFY(2 == config1.Transports().size());
    TEST_VERIFY(3 == config1.Services().size());

    NetConfig config2 = config1.Mirror(IPAddress("192.168.1.20"));
    TEST_VERIFY(CLIENT_ROLE == config2.Role());
    TEST_VERIFY(true == config2.Validate());
    TEST_VERIFY(2 == config2.Transports().size());
    TEST_VERIFY(3 == config2.Services().size());
}

IChannelListener* NetworkTest::CreateLogger(uint32 serviceId, void* context)
{
    return &logger;
}

IChannelListener* NetworkTest::CreateEcho(uint32 serviceId, void* context)
{
    if (ECHO_SERVER_CONTEXT == reinterpret_cast<intptr_t>(context))
        return &echoServer;
    else if (ECHO_CLIENT_CONTEXT == reinterpret_cast<intptr_t>(context))
        return &echoClient;
    return NULL;
}

void NetworkTest::DeleteEcho(IChannelListener* obj, void* context)
{
    // Do nothing as services are members of NetworkTest
}

void NetworkTest::DeleteLogger(IChannelListener* obj, void* context)
{
    // Do nothing as logger is member of NetworkTest
}

void NetworkTest::UpdateUI(bool waitStage, float32 left)
{
    serverBytesRecv->SetText(Format(L"%u", static_cast<uint32>(echoServer.BytesRecieved())));
    serverBytesSent->SetText(Format(L"%u", static_cast<uint32>(echoServer.BytesSent())));
    serverBytesDelivered->SetText(Format(L"%u", static_cast<uint32>(echoServer.BytesDelivered())));

    clientBytesRecv->SetText(Format(L"%u", static_cast<uint32>(echoClient.BytesRecieved())));
    clientBytesSent->SetText(Format(L"%u", static_cast<uint32>(echoClient.BytesSent())));
    clientBytesDelivered->SetText(Format(L"%u", static_cast<uint32>(echoClient.BytesDelivered())));

    if (waitStage)
    {
        timeLeft->SetText(Format(L"%.1f", left));
    }
}

void NetworkTest::CreateUI()
{
    enum {
        CONTROL_HEIGHT = 20,
        CAPTION_WIDTH = 300,
        LABEL_WIDTH = 100,
        VALUE_WIDTH = 140,
        SERVER_X = 0,
        CLIENT_X = LABEL_WIDTH + VALUE_WIDTH
    };

    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    font->SetSize(20);

    {
        UIStaticText* caption = new UIStaticText;
        caption->SetFont(font);
        caption->SetRect(Rect(0, 0, CAPTION_WIDTH, CONTROL_HEIGHT));
        caption->SetText(L"NetworkTest");
        AddControl(caption);
        SafeRelease(caption);

        UIStaticText* serverCaption = new UIStaticText;
        serverCaption->SetFont(font);
        serverCaption->SetRect(Rect(SERVER_X, CONTROL_HEIGHT, LABEL_WIDTH + VALUE_WIDTH, CONTROL_HEIGHT));
        serverCaption->SetText(L"Echo server");
        AddControl(serverCaption);
        SafeRelease(serverCaption);

        UIStaticText* clientCaption = new UIStaticText;
        clientCaption->SetFont(font);
        clientCaption->SetRect(Rect(CLIENT_X, CONTROL_HEIGHT, LABEL_WIDTH + VALUE_WIDTH, CONTROL_HEIGHT));
        clientCaption->SetText(L"Echo client");
        AddControl(clientCaption);
        SafeRelease(clientCaption);
    }
    //////////////////////////////////////////////////////////////////////////
    {
        UIStaticText* label = new UIStaticText;
        label->SetFont(font);
        label->SetRect(Rect(SERVER_X, CONTROL_HEIGHT * 2, LABEL_WIDTH, CONTROL_HEIGHT));
        label->SetText(L"recv");
        AddControl(label);
        SafeRelease(label);

        serverBytesRecv = new UIStaticText;
        serverBytesRecv->SetFont(font);
        serverBytesRecv->SetRect(Rect(SERVER_X + LABEL_WIDTH, CONTROL_HEIGHT * 2, VALUE_WIDTH, CONTROL_HEIGHT));
        serverBytesRecv->SetText(L"0");
        AddControl(serverBytesRecv);
    }
    {
        UIStaticText* label = new UIStaticText;
        label->SetFont(font);
        label->SetRect(Rect(SERVER_X, CONTROL_HEIGHT * 3, LABEL_WIDTH, CONTROL_HEIGHT));
        label->SetText(L"sent");
        AddControl(label);
        SafeRelease(label);

        serverBytesSent = new UIStaticText;
        serverBytesSent->SetFont(font);
        serverBytesSent->SetRect(Rect(SERVER_X + LABEL_WIDTH, CONTROL_HEIGHT * 3, VALUE_WIDTH, CONTROL_HEIGHT));
        serverBytesSent->SetText(L"0");
        AddControl(serverBytesSent);
    }
    {
        UIStaticText* label = new UIStaticText;
        label->SetFont(font);
        label->SetRect(Rect(SERVER_X, CONTROL_HEIGHT * 4, LABEL_WIDTH, CONTROL_HEIGHT));
        label->SetText(L"delivered");
        AddControl(label);
        SafeRelease(label);

        serverBytesDelivered = new UIStaticText;
        serverBytesDelivered->SetFont(font);
        serverBytesDelivered->SetRect(Rect(SERVER_X + LABEL_WIDTH, CONTROL_HEIGHT * 4, VALUE_WIDTH, CONTROL_HEIGHT));
        serverBytesDelivered->SetText(L"0");
        AddControl(serverBytesDelivered);
    }
    //////////////////////////////////////////////////////////////////////////
    {
        UIStaticText* label = new UIStaticText;
        label->SetFont(font);
        label->SetRect(Rect(CLIENT_X, CONTROL_HEIGHT * 2, LABEL_WIDTH, CONTROL_HEIGHT));
        label->SetText(L"recv");
        AddControl(label);
        SafeRelease(label);

        clientBytesRecv = new UIStaticText;
        clientBytesRecv->SetFont(font);
        clientBytesRecv->SetRect(Rect(CLIENT_X + LABEL_WIDTH, CONTROL_HEIGHT * 2, VALUE_WIDTH, CONTROL_HEIGHT));
        clientBytesRecv->SetText(L"0");
        AddControl(clientBytesRecv);
    }
    {
        UIStaticText* label = new UIStaticText;
        label->SetFont(font);
        label->SetRect(Rect(CLIENT_X, CONTROL_HEIGHT * 3, LABEL_WIDTH, CONTROL_HEIGHT));
        label->SetText(L"sent");
        AddControl(label);
        SafeRelease(label);

        clientBytesSent = new UIStaticText;
        clientBytesSent->SetFont(font);
        clientBytesSent->SetRect(Rect(CLIENT_X + LABEL_WIDTH, CONTROL_HEIGHT * 3, VALUE_WIDTH, CONTROL_HEIGHT));
        clientBytesSent->SetText(L"0");
        AddControl(clientBytesSent);
    }
    {
        UIStaticText* label = new UIStaticText;
        label->SetFont(font);
        label->SetRect(Rect(CLIENT_X, CONTROL_HEIGHT * 4, LABEL_WIDTH, CONTROL_HEIGHT));
        label->SetText(L"delivered");
        AddControl(label);
        SafeRelease(label);

        clientBytesDelivered = new UIStaticText;
        clientBytesDelivered->SetFont(font);
        clientBytesDelivered->SetRect(Rect(CLIENT_X + LABEL_WIDTH, CONTROL_HEIGHT * 4, VALUE_WIDTH, CONTROL_HEIGHT));
        clientBytesDelivered->SetText(L"0");
        AddControl(clientBytesDelivered);
    }
    //////////////////////////////////////////////////////////////////////////
    {
        timeLeft = new UIStaticText;
        timeLeft->SetFont(font);
        timeLeft->SetRect(Rect(SERVER_X, CONTROL_HEIGHT * 5, VALUE_WIDTH, CONTROL_HEIGHT));
        AddControl(timeLeft);
    }
    SafeRelease(font);
}

void NetworkTest::DestroyUI()
{
    SafeRelease(serverBytesRecv);
    SafeRelease(serverBytesSent);
    SafeRelease(serverBytesDelivered);
    SafeRelease(clientBytesRecv);
    SafeRelease(clientBytesSent);
    SafeRelease(clientBytesDelivered);
    SafeRelease(timeLeft);
}
