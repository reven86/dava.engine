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

#if defined(__DAVAENGINE_ANDROID__)
#include <Utils/UTF8Utils.h>
#endif

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
                           , serviceCreatorStage(0)
{
    new NetCore();

    NetCore::Instance()->RegisterService(SERVICE_LOG, MakeFunction(this, &NetworkTest::CreateLogger), MakeFunction(this, &NetworkTest::DeleteService));
    NetCore::Instance()->RegisterService(SERVICE_ECHO, MakeFunction(this, &NetworkTest::CreateEcho), MakeFunction(this, &NetworkTest::DeleteService));

    RegisterFunction(this, &NetworkTest::TestEcho, "TestEcho", NULL);
    RegisterFunction(this, &NetworkTest::TestIPAddress, "TestIPAddress", NULL);
    RegisterFunction(this, &NetworkTest::TestEndpoint, "TestEndpoint", NULL);
    RegisterFunction(this, &NetworkTest::TestNetConfig, "TestNetConfig", NULL);
}

NetworkTest::~NetworkTest()
{
    NetCore::Instance()->Release();
}

void NetworkTest::LoadResources()
{
    {
        NetConfig loggerConfig(SERVER_ROLE);
        loggerConfig.AddTransport(TRANSPORT_TCP, Endpoint(LOGGER_PORT));
        loggerConfig.AddService(SERVICE_LOG);

        NetCore::Instance()->CreateDriver(loggerConfig);
    }
    {
#if defined(__DAVAENGINE_ANDROID__)
    	// Cannot log wide string directly, maybe due to incorrect format flag
    	// I tried %s and %ls for wide string
        Logger::Debug( "Name        : %s", UTF8Utils::EncodeToUTF8(DeviceInfo::GetName()).c_str());
#elif defined(__DAVAENGINE_WIN32__)
    	Logger::Debug(L"Name        : %s", DeviceInfo::GetName().c_str());
#else
        Logger::Debug(L"Name        : %ls", DeviceInfo::GetName().c_str());
#endif
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
    NetCore::Instance()->CreateDriver(serverConfig);
    NetCore::Instance()->CreateDriver(clientConfig);
}

void NetworkTest::UnloadResources()
{
    NetCore::Instance()->Finish(true);
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
    }
    TestTemplate<NetworkTest>::Update(timeElapsed);

    NetCore::Instance()->Poll();
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
    IPAddress addrEmpty;
    TEST_VERIFY(true == addrEmpty.IsUnspecified());
    TEST_VERIFY(0 == addrEmpty.ToUInt());
    TEST_VERIFY("0.0.0.0" == addrEmpty.ToString());

    IPAddress addrMulticast = IPAddress::FromString("239.192.100.1");
    TEST_VERIFY(false == addrMulticast.IsUnspecified());
    TEST_VERIFY(true == addrMulticast.IsMulticast());
    TEST_VERIFY("239.192.100.1" == addrMulticast.ToString());

    IPAddress addrSame1("192.168.0.4");
    IPAddress addrSame2 = IPAddress::FromString("192.168.0.4");
    TEST_VERIFY(false == addrSame1.IsUnspecified());
    TEST_VERIFY(false == addrSame1.IsMulticast());
    TEST_VERIFY("192.168.0.4" == addrSame1.ToString());
    TEST_VERIFY(addrSame1 == addrSame2);
}

void NetworkTest::TestEndpoint(PerfFuncData* data)
{
    Endpoint endpoint1;
    TEST_VERIFY(0 == endpoint1.Port());
    TEST_VERIFY("0.0.0.0:0" == endpoint1.ToString());

    Endpoint endpoint2("192.168.1.45", 1234);
    TEST_VERIFY(1234 == endpoint2.Port());
    TEST_VERIFY(endpoint2.Address() == IPAddress::FromString("192.168.1.45"));

    Endpoint endpoint3(IPAddress("192.168.1.45"), 1234);
    TEST_VERIFY(endpoint3.Address() == IPAddress("192.168.1.45"));
    TEST_VERIFY(endpoint2 == endpoint3);
}

void NetworkTest::TestNetConfig(PerfFuncData* data)
{
    NetConfig config_empty(SERVER_ROLE);
    TEST_VERIFY(SERVER_ROLE == config_empty.Role());
    TEST_VERIFY(false == config_empty.Validate());

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

IChannelListener* NetworkTest::CreateLogger(uint32 serviceId)
{
    return SERVICE_LOG == serviceId ? &logger
                                    : NULL;
}

IChannelListener* NetworkTest::CreateEcho(uint32 serviceId)
{
    IChannelListener* obj = NULL;
    if (SERVICE_ECHO == serviceId)
    {
        if (0 == serviceCreatorStage)
            obj = &echoServer;
        else if (1 == serviceCreatorStage)
            obj = &echoClient;
        serviceCreatorStage += 1;
    }
    return obj;
}

void NetworkTest::DeleteService(IChannelListener* obj)
{
    // Do nothing as services are created on stack
}
