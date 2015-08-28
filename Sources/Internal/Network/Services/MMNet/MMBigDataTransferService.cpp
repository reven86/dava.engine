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

#include "Functional/Function.h"

#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/Logger.h"

#include "Concurrency/Thread.h"

#include "Network/NetConfig.h"
#include "Network/ServiceRegistrar.h"
#include "Network/Base/IOLoop.h"
#include "Network/Private/NetController.h"
#include "Network/Services/MMNet/MMNetProto.h"
#include "Network/Services/MMNet/MMBigDataTransferService.h"

namespace DAVA
{
namespace Net
{

MMBigDataTransferService::MMBigDataTransferService(eNetworkRole role_)
    : role(role_)
    , ioLoop(new IOLoop(false))
    , registrar(new ServiceRegistrar)
    , ioThread(Thread::Create([this]() { IOThread(); }))
{
    registrar->Register(SERVICE_ID, MakeFunction(this, &MMBigDataTransferService::NetServiceCreator),
                                    MakeFunction(this, &MMBigDataTransferService::NetServiceDeleter));
    ioThread->Start();
}

MMBigDataTransferService::~MMBigDataTransferService()
{
    if (netController != nullptr)
    {
        netController->Stop(MakeFunction(this, &MMBigDataTransferService::OnNetControllerStopped));
    }
    ioLoop->PostQuit();
    ioThread->Join();
}

void MMBigDataTransferService::Start(bool newSession, uint32 connToken_, const IPAddress& addr)
{
    connToken = connToken_;
    if (newSession && SERVER_ROLE == role)
    {
        for (SnapshotInfo& o : snapshotQueue)
        {
            FileSystem::Instance()->DeleteFile(o.filename);
        }
        snapshotQueue.clear();
    }

    NetConfig config(role);
    Endpoint endpoint = SERVER_ROLE == role ? Endpoint(PORT)
                                            : Endpoint(addr, PORT);

    config.AddTransport(TRANSPORT_TCP, endpoint);
    config.AddService(SERVICE_ID);

    netController.reset(new NetController(ioLoop.get(), *registrar, this));
    netController->ApplyConfig(config);
    netController->Start();
}

void MMBigDataTransferService::Stop()
{
    if (netController != nullptr)
    {
        netController->Stop(MakeFunction(this, &MMBigDataTransferService::OnNetControllerStopped));
    }
}

void MMBigDataTransferService::TransferSnapshot(const FilePath& snapshotFile)
{
    DVASSERT(SERVER_ROLE == role);

    ioLoop->Post(Bind(MakeFunction(this, &MMBigDataTransferService::DoTransferSnapshot), snapshotFile));
}

void MMBigDataTransferService::SetSnapshotCallback(SnapshotCallback callback)
{
    DVASSERT(CLIENT_ROLE == role);
    DVASSERT(callback != nullptr);

    snapshotCallback = callback;
}

void MMBigDataTransferService::ChannelOpen()
{
}

void MMBigDataTransferService::ChannelClosed(const char8* /*message*/)
{
    SafeRelease(fileHandle);
    if (!snapshotQueue.empty())
    {
        SnapshotInfo* snapshot = &snapshotQueue.front();
        snapshot->chunkSize = 0;
        snapshot->bytesTransferred = 0;
    }
}

void MMBigDataTransferService::PacketReceived(const void* packet, size_t length)
{
    if (CLIENT_ROLE == role)
    {
        ClientPacketRecieved(packet, length);
    }
}

void MMBigDataTransferService::PacketDelivered()
{
    if (SERVER_ROLE == role)
    {
        ServerPacketDelivered();
    }
}

void MMBigDataTransferService::ServerPacketDelivered()
{
    DVASSERT(!snapshotQueue.empty());

    SnapshotInfo* snapshot = &snapshotQueue.front();
    snapshot->bytesTransferred += snapshot->chunkSize;

    if (snapshot->bytesTransferred == snapshot->fileSize)
    {
        SafeRelease(fileHandle);
        FileSystem::Instance()->DeleteFile(snapshot->filename);
        snapshotQueue.pop_front();

        while (!snapshotQueue.empty())
        {
            snapshot = &snapshotQueue.front();
            if (!BeginNextSnapshot(snapshot))
            {
                FileSystem::Instance()->DeleteFile(snapshot->filename);
                snapshotQueue.pop_front();
            }
            else
                break;
        }
    }
    else
    {
        SendNextChunk(snapshot);
    }
}

void MMBigDataTransferService::DoTransferSnapshot(const FilePath& snapshotFile)
{
    bool wasEmpty = snapshotQueue.empty();
    snapshotQueue.emplace_back(SnapshotInfo(snapshotFile));
    if (wasEmpty)
    {
        SnapshotInfo* snapshot = &snapshotQueue.front();
        BeginNextSnapshot(snapshot);
    }
}

void MMBigDataTransferService::SendNextChunk(SnapshotInfo* snapshot)
{
    MMNetProto::PacketHeader* hdr = OffsetPointer<MMNetProto::PacketHeader>(outbuf.data(), 0);
    MMNetProto::PacketParamSnapshot* param = OffsetPointer<MMNetProto::PacketParamSnapshot>(outbuf.data(), sizeof(MMNetProto::PacketHeader));

    const uint32 PACKET_PREFIX_SIZE = sizeof(MMNetProto::PacketHeader) + sizeof(MMNetProto::PacketParamSnapshot);
    const uint32 chunkSize = std::min(OUTBUF_SIZE - PACKET_PREFIX_SIZE, snapshot->fileSize - snapshot->bytesTransferred);
    uint32 nread = fileHandle->Read(outbuf.data() + PACKET_PREFIX_SIZE, chunkSize);
    if (nread == chunkSize)
    {
        snapshot->chunkSize = chunkSize;

        hdr->length = PACKET_PREFIX_SIZE + chunkSize;
        hdr->type = MMNetProto::TYPE_AUTO_SNAPSHOT;
        hdr->status = MMNetProto::STATUS_SUCCESS;
        hdr->itemCount = 0;
        hdr->token = connToken;

        param->flags = 0;
        param->snapshotSize = snapshot->fileSize;
        param->chunkSize = snapshot->chunkSize;
        param->chunkOffset = snapshot->bytesTransferred;
    }
    else
    {
        snapshot->bytesTransferred = 0;
        snapshot->fileSize = chunkSize;

        hdr->length = sizeof(MMNetProto::PacketHeader);
        hdr->type = MMNetProto::TYPE_AUTO_SNAPSHOT;
        hdr->status = MMNetProto::STATUS_ERROR;
        hdr->itemCount = 0;
        hdr->token = connToken;
    }
    Send(outbuf.data(), hdr->length);
}

bool MMBigDataTransferService::BeginNextSnapshot(SnapshotInfo* snapshot)
{
    fileHandle = File::Create(snapshot->filename, File::OPEN | File::READ);
    if (fileHandle != nullptr)
    {
        snapshot->fileSize = fileHandle->GetSize();
        if (snapshot->fileSize > 0)
        {
            SendNextChunk(snapshot);
            return true;
        }
        SafeRelease(fileHandle);
    }
    return false;
}

void MMBigDataTransferService::ClientPacketRecieved(const void* packet, size_t length)
{
    const size_t dataLength = length - sizeof(MMNetProto::PacketHeader);
    const MMNetProto::PacketHeader* header = static_cast<const MMNetProto::PacketHeader*>(packet);
    if (length >= sizeof(MMNetProto::PacketHeader) && header->length == length)
    {
        switch (header->type)
        {
        case MMNetProto::TYPE_AUTO_SNAPSHOT:
            ProcessAutoReplySnapshot(header, static_cast<const void*>(header + 1), dataLength);
            break;
        default:
            break;
        }
    }
}

void MMBigDataTransferService::ProcessAutoReplySnapshot(const MMNetProto::PacketHeader* inHeader, const void* packetData, size_t dataLength)
{
    uint32 totalSize = 0;
    uint32 chunkOffset = 0;
    uint32 chunkSize = 0;
    const uint8* chunk = nullptr;

    if (inHeader->status == MMNetProto::STATUS_SUCCESS)
    {
        const MMNetProto::PacketParamSnapshot* param = static_cast<const MMNetProto::PacketParamSnapshot*>(packetData);
        totalSize = param->snapshotSize;
        chunkOffset = param->chunkOffset;
        chunkSize = param->chunkSize;
        chunk = OffsetPointer<const uint8>(param, sizeof(MMNetProto::PacketParamSnapshot));
    }
    snapshotCallback(totalSize, chunkOffset, chunkSize, chunk);
}

void MMBigDataTransferService::IOThread()
{
    ioLoop->Run();
}

void MMBigDataTransferService::OnNetControllerStopped(IController* controller)
{
    netController.reset();
}

IChannelListener* MMBigDataTransferService::NetServiceCreator(uint32 serviceId, void* context)
{
    if (!netServiceInUse)
    {
        netServiceInUse = true;
        return this;
    }
    return nullptr;
}

void MMBigDataTransferService::NetServiceDeleter(IChannelListener* service, void* context)
{
    netServiceInUse = false;
}

}   // namespace Net
}   // namespace DAVA
