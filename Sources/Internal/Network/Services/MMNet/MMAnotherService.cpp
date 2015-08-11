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

#include "Base/FunctionTraits.h"
#include "FileSystem/Logger.h"

#include "Concurrency/Thread.h"

#include "Network/NetConfig.h"
#include "Network/ServiceRegistrar.h"
#include "Network/Base/IOLoop.h"
#include "Network/Private/NetController.h"
#include "Network/Services/MMNet/MMAnotherService.h"

namespace DAVA
{
namespace Net
{

MMAnotherService::MMAnotherService(eNetworkRole role_)
    : role(role_)
    , ioLoop(new IOLoop(false))
    , registrar(new ServiceRegistrar)
    , ioThread(Thread::Create([this]() { IOThread(); }))
{
    registrar->Register(SERVICE_ID, MakeFunction(this, &MMAnotherService::NetServiceCreator),
                                    MakeFunction(this, &MMAnotherService::NetServiceDeleter));
    ioThread->Start();
}

MMAnotherService::~MMAnotherService()
{
    if (netController != nullptr)
    {
        netController->Stop(MakeFunction(this, &MMAnotherService::OnNetControllerStopped));
    }
    ioLoop->PostQuit();
    ioThread->Join();
}

void MMAnotherService::Start(uint32 connToken_, const IPAddress& addr)
{
    connToken = connToken_;

    NetConfig config(role);
    Endpoint endpoint = SERVER_ROLE == role ? Endpoint(PORT)
                                            : Endpoint(addr, PORT);

    config.AddTransport(TRANSPORT_TCP, endpoint);
    config.AddService(SERVICE_ID);

    netController.reset(new NetController(ioLoop.get(), *registrar, this));
    netController->ApplyConfig(config);
    netController->Start();
}

void MMAnotherService::Stop()
{
    netController->Stop(MakeFunction(this, &MMAnotherService::OnNetControllerStopped));
}

void MMAnotherService::ChannelOpen()
{

}

void MMAnotherService::ChannelClosed(const char8* message)
{

}

void MMAnotherService::PacketReceived(const void* packet, size_t length)
{

}

void MMAnotherService::PacketDelivered()
{

}

void MMAnotherService::IOThread()
{
    Logger::Debug("*************** MMAnotherService::IOThread enter: %u", GetCurrentThreadId());
    ioLoop->Run();
    Logger::Debug("*************** MMAnotherService::IOThread leave");
}

void MMAnotherService::OnNetControllerStopped(IController* controller)
{
    netController.reset();
}

IChannelListener* MMAnotherService::NetServiceCreator(uint32 serviceId, void* context)
{
    if (!netServiceInUse)
    {
        netServiceInUse = true;
        return this;
    }
    return nullptr;
}

void MMAnotherService::NetServiceDeleter(IChannelListener* service, void* context)
{
    netServiceInUse = false;
}

}   // namespace Net
}   // namespace DAVA
