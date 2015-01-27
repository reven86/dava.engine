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

#ifndef __NETWORK_TEST_H__
#define __NETWORK_TEST_H__

#include <DAVAEngine.h>

#include <Network/NetService.h>

#include "TestTemplate.h"

//#define NETWORKTEST_WITH_UI_FOR_LOCAL

using DAVA::Net::IChannel;
using DAVA::Net::IChannelListener;

struct Parcel
{
    void* outbuf;
    size_t length;
    uint32 packetId;

    friend bool operator == (const Parcel& o, const void* p) { return o.outbuf == p; }
};

class TestEchoServer : public DAVA::Net::NetService
{
public:
    TestEchoServer();

    virtual void OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length);
    virtual void OnPacketSent(IChannel* aChannel, const void* buffer, size_t length);
    virtual void OnPacketDelivered(IChannel* aChannel, uint32 packetId);

    bool IsTestDone() const { return testDone; }

    size_t BytesRecieved() const { return bytesRecieved; }
    size_t BytesSent() const { return bytesSent; }
    size_t BytesDelivered() const { return bytesDelivered; }

private:
    void SendEcho(const void* buffer, size_t length);

private:
    bool testDone;
    size_t bytesRecieved;
    size_t bytesSent;
    size_t bytesDelivered;
    uint32 lastPacketId;

    Deque<Parcel> parcels;
};

//////////////////////////////////////////////////////////////////////////
class TestEchoClient : public DAVA::Net::NetService
{
public:
    TestEchoClient();
    virtual ~TestEchoClient();

    virtual void ChannelOpen();
    virtual void OnPacketReceived(IChannel* aChannel, const void* buffer, size_t length);
    virtual void OnPacketSent(IChannel* aChannel, const void* buffer, size_t length);
    virtual void OnPacketDelivered(IChannel* aChannel, uint32 packetId);

    bool IsTestDone() const { return testDone; }

    size_t BytesRecieved() const { return bytesRecieved; }
    size_t BytesSent() const { return bytesSent; }
    size_t BytesDelivered() const { return bytesDelivered; }

private:
    void SendParcel(Parcel* parcel);

private:
    bool testDone;
    size_t bytesRecieved;
    size_t bytesSent;
    size_t bytesDelivered;

    Deque<Parcel> parcels;
    size_t pendingRead;         // Parcel index expected to be read from server
    size_t pendingSent;         // Parcel index expected to be sent
    size_t pendingDelivered;    // Parcel index expected to be confirmed as delivered
};

//////////////////////////////////////////////////////////////////////////
class NetworkTest : public TestTemplate<NetworkTest>
{
protected:
    ~NetworkTest();

    enum eServiceTypes
    {
        SERVICE_ECHO
    };

    enum
    {
        ECHO_SERVER_CONTEXT,
        ECHO_CLIENT_CONTEXT
    };

    static const uint16 ECHO_PORT = 9999;

public:
    NetworkTest();

    virtual void LoadResources();
    virtual void UnloadResources();
    virtual bool RunTest(int32 testNum);

    virtual void Update(float32 timeElapsed);

    void TestEcho(PerfFuncData* data);
    void TestIPAddress(PerfFuncData* data);
    void TestEndpoint(PerfFuncData* data);
    void TestNetConfig(PerfFuncData* data);

    IChannelListener* CreateEcho(uint32 serviceId, void* context);
    void DeleteEcho(IChannelListener* obj, void* context);

#ifdef NETWORKTEST_WITH_UI_FOR_LOCAL
private:
    void CreateUI();
    void UpdateUI();
    void DestroyUI();
#endif  // NETWORKTEST_WITH_UI_FOR_LOCAL

private:
    bool testingEcho;
    TestEchoServer echoServer;
    TestEchoClient echoClient;

#ifdef NETWORKTEST_WITH_UI_FOR_LOCAL
    DAVA::UIStaticText* serverBytesRecv;
    DAVA::UIStaticText* serverBytesSent;
    DAVA::UIStaticText* serverBytesDelivered;
    DAVA::UIStaticText* clientBytesRecv;
    DAVA::UIStaticText* clientBytesSent;
    DAVA::UIStaticText* clientBytesDelivered;
#endif  // NETWORKTEST_WITH_UI_FOR_LOCAL
};

#endif  // __NETWORK_TEST_H
