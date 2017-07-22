#ifndef __SERVER_LOGICS_H__
#define __SERVER_LOGICS_H__

#include "CacheDB.h"

#include <Tools/AssetCache/AssetCache.h>

class ServerLogics : public DAVA::AssetCache::ServerNetProxyListener,
                     public DAVA::AssetCache::ClientNetProxyListener
{
public:
    void Init(DAVA::AssetCache::ServerNetProxy* server, const DAVA::String& serverName, DAVA::AssetCache::ClientNetProxy* client, CacheDB* dataBase);

    //ServerNetProxyListener
    void OnAddToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::AssetCache::CachedItemValue&& value) override;
    void OnRequestedFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnRemoveFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnClearCache(DAVA::Net::IChannel* channel) override;
    void OnWarmingUp(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnChannelClosed(DAVA::Net::IChannel* channel, const DAVA::char8* message) override;
    void OnStatusRequested(DAVA::Net::IChannel* channel) override;

    //ClientNetProxyListener
    void OnClientProxyStateChanged() override;
    void OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::AssetCache::CachedItemValue& value) override;

    void OnRemoteDisconnecting();

    void Update();

private:
    bool IsRemoteServerConnected() const;

    template <typename... Args>
    void AddServerTask(Args&&... args);

    void ProcessServerTasks();

private:
    DAVA::AssetCache::ServerNetProxy* serverProxy = nullptr;
    DAVA::AssetCache::ClientNetProxy* clientProxy = nullptr;
    CacheDB* dataBase = nullptr;

    struct RequestDescription
    {
        RequestDescription(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& _key, DAVA::AssetCache::ePacketID _request);

        DAVA::Net::IChannel* clientChannel = nullptr;
        DAVA::AssetCache::CacheItemKey key;

        DAVA::AssetCache::ePacketID request = DAVA::AssetCache::PACKET_UNKNOWN;
    };

    DAVA::List<RequestDescription> waitedRequests;

    struct ServerTask
    {
        ServerTask(const DAVA::AssetCache::CacheItemKey& _key, DAVA::AssetCache::ePacketID _request);
        ServerTask(const DAVA::AssetCache::CacheItemKey& _key, DAVA::AssetCache::CachedItemValue&& _value, DAVA::AssetCache::ePacketID _request);

        DAVA::AssetCache::CacheItemKey key;
        DAVA::AssetCache::CachedItemValue value;

        DAVA::AssetCache::ePacketID request = DAVA::AssetCache::PACKET_UNKNOWN;
    };

    DAVA::List<ServerTask> serverTasks;
    DAVA::String serverName;
};

template <typename... Args>
void ServerLogics::AddServerTask(Args&&... args)
{
    if (IsRemoteServerConnected())
    {
        serverTasks.emplace_back(std::forward<Args>(args)...);
    }
}

#endif // __SERVER_LOGICS_H__
