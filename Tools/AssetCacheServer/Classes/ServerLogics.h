#ifndef __SERVER_LOGICS_H__
#define __SERVER_LOGICS_H__

#include "AssetCache/AssetCache.h"

class ServerLogics : public DAVA::AssetCache::ServerNetProxyListener,
                     public DAVA::AssetCache::ClientNetProxyListener
{
public:
    void Init(DAVA::AssetCache::ServerNetProxy* server, const DAVA::String& serverName, DAVA::AssetCache::ClientNetProxy* client, DAVA::AssetCache::CacheDB* dataBase);

    //ServerNetProxyListener
    void OnAddToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::AssetCache::CachedItemValue&& value) override;
    void OnRequestedFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnWarmingUp(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key) override;
    void OnChannelClosed(DAVA::Net::IChannel* channel, const DAVA::char8* message) override;

    //ClientNetProxyListener
    void OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::AssetCache::CachedItemValue& value) override;

    void Update();

private:
    void ProcessServerTasks();

private:
    DAVA::AssetCache::ServerNetProxy* server = nullptr;
    DAVA::AssetCache::ClientNetProxy* client = nullptr;
    DAVA::AssetCache::CacheDB* dataBase = nullptr;

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

#endif // __SERVER_LOGICS_H__
