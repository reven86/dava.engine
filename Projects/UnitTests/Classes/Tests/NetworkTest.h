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

#include <Network/PeerDesription.h>
#include <Network/NetService.h>
#include <Network/Services/NetLogger.h>
#include <Network/NetCore.h>

#include "MemoryManager/MemoryManagerAllocator.h"

#include "TestTemplate.h"

using DAVA::Net::IChannel;
using DAVA::Net::IChannelListener;

class NetworkTest : public TestTemplate<NetworkTest>
{
protected:
    ~NetworkTest();

    enum eServiceTypes
    {
        SERVICE_LOG,
    };

    static const uint16 ANNOUNCE_PORT = 9999;
    static const uint32 ANNOUNCE_TIME_PERIOD = 5;
    static const char8 announceMulticastGroup[];

    static const uint16 LOGGER_PORT = 9999;

public:
    NetworkTest();

    virtual void LoadResources();
    virtual void UnloadResources();
    virtual bool RunTest(int32 testNum);

    virtual void Update(float32 timeElapsed);
    void ButtonPressed(BaseObject *obj, void *data, void *callerData);

    void DummyTest(PerfFuncData* data);

    IChannelListener* CreateLogger(uint32 serviceId, void* context);
    void DeleteLogger(IChannelListener* obj, void* context);

private:
    void CreateUI();
    DAVA::UIButton* CreateButton(const wchar_t* caption, const DAVA::Rect& rc, DAVA::Font* font);
    void DestroyUI();

    size_t AnnounceDataSupplier(size_t length, void* buffer);

private:
    bool quitFlag;
    bool periodicFlag;
    bool loggerInUse;
    DAVA::Net::NetLogger logger;

    //DAVA::Vector<char> v;
    std::vector<char, DAVA::MemoryManagerAllocator<char, 2>> v1;
    std::vector<char, DAVA::MemoryManagerAllocator<char, 3>> v2;

    DAVA::Net::PeerDescription peerDescr;
    Vector<uint8> peerDescrSerialized;

    DAVA::UIButton* btnDebug;
    DAVA::UIButton* btnInfo;
    DAVA::UIButton* btnWarn;
    DAVA::UIButton* btnError;
    DAVA::UIButton* btnPacket;
    DAVA::UIButton* btnPeriodic;
    DAVA::UIButton* btnRestart;
    DAVA::UIButton* btnQuit;
};

#endif  // __NETWORK_TEST_H
