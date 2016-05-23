#ifndef __ASSET_CACHE_CLIENT_H__
#define __ASSET_CACHE_CLIENT_H__

#include "AssetCache/AssetCache.h"
#include <atomic>

namespace DAVA
{
class AssetCacheClient final : public AssetCache::ClientNetProxyListener
{
    struct Request
    {
        Request() = default;
        Request(const AssetCache::CacheItemKey& key_, AssetCache::CachedItemValue* value_, AssetCache::ePacketID requestID_)
            : key(key_)
            , value(value_)
            , requestID(requestID_)
            , result(AssetCache::Error::CODE_NOT_INITIALIZED)
            , recieved(false)
            , processingRequest(false)
        {
        }

        void Reset()
        {
            value = nullptr;

            requestID = AssetCache::PACKET_UNKNOWN;
            result = AssetCache::Error::CODE_NOT_INITIALIZED;

            recieved = false;
            processingRequest = false;
        }

        AssetCache::CacheItemKey key;
        AssetCache::CachedItemValue* value = nullptr;

        AssetCache::ePacketID requestID = AssetCache::PACKET_UNKNOWN;
        AssetCache::Error result = AssetCache::Error::NO_ERRORS;

        bool recieved = false;
        bool processingRequest = false;
    };

public:
    struct ConnectionParams
    {
        String ip = AssetCache::LOCALHOST;
        uint16 port = AssetCache::ASSET_SERVER_PORT;
        uint64 timeoutms = 60 * 1000;
    };

    AssetCacheClient(bool emulateNetworkLoop);
    ~AssetCacheClient() override;

    AssetCache::Error ConnectSynchronously(const ConnectionParams& connectionParams);
    void Disconnect();

    AssetCache::Error AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value);
    AssetCache::Error RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value);

    bool IsConnected() const;

private:
    void ProcessNetwork();

    AssetCache::Error WaitRequest();

    //ClientNetProxyListener
    void OnAddedToCache(const AssetCache::CacheItemKey& key, bool added) override;
    void OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value) override;
    void OnAssetClientStateChanged() override;

private:
    AssetCache::ClientNetProxy client;

    uint64 timeoutms = 60u * 1000u;

    Mutex requestLocker;
    Request request;

    std::atomic<bool> isActive;
    std::atomic<bool> isJobStarted;

    bool emulateNetworkLoop = false;
};

} //END of DAVA

#endif //__ASSET_CACHE_CLIENT_H__
