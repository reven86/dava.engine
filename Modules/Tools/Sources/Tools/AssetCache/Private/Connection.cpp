#include "Tools/AssetCache/Connection.h"
#include "Tools/AssetCache/AssetCacheConstants.h"

#include <Debug/DVAssert.h>
#include <FileSystem/KeyedArchive.h>
#include <Logger/Logger.h>
#include <Base/StaticSingleton.h>

#include <Network/IChannel.h>
#include <Network/NetworkCommon.h>
#include <Network/NetConfig.h>
#include <Network/NetCore.h>
#include <Network/Base/Endpoint.h>

#include <Concurrency/LockGuard.h>

namespace DAVA
{
namespace AssetCache
{
namespace ConnectionDetails
{
/**
    ControllerHolder allows to hold instances of ChannelListenerDispatcher on shared pointer.

    supposed flow:
    1. create instance of ChannelListenerDispatcher. Instance is holded by Connection object by shared pointer
    2. register ChannelListenerDispatcher as a controller in netcore
    3. add it to ControllersHolder. Now two objects are holding ChannelListenerDispatcher instance
    4. remove controller, specify callback function ControllersHolder::RemoveController
    5. delete Connection
    6. as soon as controller is removed from netcore, callback RemoveController will be invoked, that will finally release ChannelListenerDispatcher object
*/
class ControllersHolder : public StaticSingleton<ControllersHolder>
{
public:
    void AddController(Net::NetCore::TrackId controllerId, std::shared_ptr<Net::ChannelListenerDispatched>& controller)
    {
        controllers.emplace(controllerId, controller);
    }

    void RemoveController(Net::NetCore::TrackId controllerId)
    {
        controllers.erase(controllerId);
    }

private:
    UnorderedMap<Net::NetCore::TrackId, std::shared_ptr<Net::ChannelListenerDispatched>> controllers;
};
}

bool SendArchieve(Net::IChannel* channel, KeyedArchive* archieve)
{
    DVASSERT(archieve && channel);

    auto packedSize = archieve->Save(nullptr, 0);
    uint8* packedData = new uint8[packedSize];

    const uint32 serializedSize = archieve->Save(packedData, packedSize);
    DVASSERT(packedSize == serializedSize);

    uint32 packedId = 0;
    return channel->Send(packedData, packedSize, 0, &packedId);
}

std::shared_ptr<Connection> Connection::MakeConnection(
Dispatcher<Function<void()>>* dispatcher,
Net::eNetworkRole role,
const Net::Endpoint& endpoint,
Net::IChannelListener* listener,
Net::eTransportType transport,
uint32 timeoutMs)
{
    std::shared_ptr<Connection> connection(new Connection(dispatcher, role, endpoint, listener, transport, timeoutMs));
    bool res = connection->Connect(role, transport, timeoutMs);
    if (!res)
    {
        connection.reset();
    }
    return connection;
}

Connection::Connection(Dispatcher<Function<void()>>* dispatcher, Net::eNetworkRole _role, const Net::Endpoint& _endpoint, Net::IChannelListener* _listener, Net::eTransportType transport, uint32 timeoutMs)
    : dispatcher(dispatcher)
    , endpoint(_endpoint)
    , listener(_listener)
{
}

Connection::~Connection()
{
    listener = nullptr;
    if (Net::NetCore::INVALID_TRACK_ID != controllerId && Net::NetCore::Instance() != nullptr)
    {
        Disconnect();
    }
}

bool Connection::Connect(Net::eNetworkRole role, Net::eTransportType transport, uint32 timeoutMs)
{
    channelListenerDispatched.reset(new Net::ChannelListenerDispatched(shared_from_this(), dispatcher));

    const auto serviceID = NET_SERVICE_ID;

    bool isRegistered = Net::NetCore::Instance()->IsServiceRegistered(serviceID);
    if (!isRegistered)
    {
        isRegistered = Net::NetCore::Instance()->RegisterService(serviceID,
                                                                 MakeFunction(&Connection::Create),
                                                                 MakeFunction(&Connection::Delete));
    }

    if (isRegistered)
    {
        Net::NetConfig config(role);
        config.AddTransport(transport, endpoint);
        config.AddService(serviceID);

        controllerId = Net::NetCore::Instance()->CreateController(config, channelListenerDispatched.get(), timeoutMs);
        if (Net::NetCore::INVALID_TRACK_ID != controllerId)
        {
            ConnectionDetails::ControllersHolder::Instance()->AddController(controllerId, channelListenerDispatched);
            return true;
        }
        else
        {
            Logger::Error("[TCPConnection::%s] Cannot create controller", __FUNCTION__);
        }
    }
    else
    {
        Logger::Error("[TCPConnection::%s] Cannot register service(%d)", __FUNCTION__, NET_SERVICE_ID);
    }

    return false;
}

void Connection::Disconnect()
{
    DVASSERT(Net::NetCore::INVALID_TRACK_ID != controllerId);
    DVASSERT(Net::NetCore::Instance() != nullptr);

    listener = nullptr;

    Function<void()> onDestroyedCallback = Bind(&ConnectionDetails::ControllersHolder::RemoveController, ConnectionDetails::ControllersHolder::Instance(), controllerId);
    Net::NetCore::Instance()->DestroyController(controllerId, onDestroyedCallback);
    controllerId = Net::NetCore::INVALID_TRACK_ID;
}

void Connection::DisconnectBlocked()
{
    DVASSERT(Net::NetCore::INVALID_TRACK_ID != controllerId);
    DVASSERT(Net::NetCore::Instance() != nullptr);

    listener = nullptr;
    Net::NetCore::Instance()->DestroyControllerBlocked(controllerId);
    controllerId = Net::NetCore::INVALID_TRACK_ID;
}

Net::IChannelListener* Connection::Create(uint32 serviceId, void* context)
{
    Net::IChannelListener* connection = static_cast<Net::IChannelListener*>(context);
    return connection;
}

void Connection::Delete(Net::IChannelListener* obj, void* context)
{
    //do nothing
    //listener has external creation and deletion
}

void Connection::OnChannelOpen(Net::IChannel* channel)
{
    if (listener != nullptr)
    {
        listener->OnChannelOpen(channel);
    }
}

void Connection::OnChannelClosed(Net::IChannel* channel, const char8* message)
{
    if (listener != nullptr)
    {
        listener->OnChannelClosed(channel, message);
    }
}

void Connection::OnPacketReceived(Net::IChannel* channel, const void* buffer, size_t length)
{
    if (listener != nullptr)
    {
        listener->OnPacketReceived(channel, buffer, length);
    }
}

void Connection::OnPacketSent(Net::IChannel* channel, const void* buffer, size_t length)
{
    if (listener != nullptr)
    {
        listener->OnPacketSent(channel, buffer, length);
    }
}

void Connection::OnPacketDelivered(Net::IChannel* channel, uint32 packetId)
{
    if (listener != nullptr)
    {
        listener->OnPacketDelivered(channel, packetId);
    }
}

} // end of namespace AssetCache
} // end of namespace DAVA
