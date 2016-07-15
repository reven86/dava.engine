#ifndef __ASSET_CACHE_CLIENT_H__
#define __ASSET_CACHE_CLIENT_H__

#include "AssetCache.h"
#include "Base/Introspection.h"
#include "Preferences/PreferencesRegistrator.h"
#include <atomic>

namespace DAVA
{
class AssetCacheClient final : public AssetCache::ClientNetProxyListener
{
public:
    struct ConnectionParams : InspBase
    {
        String ip = AssetCache::GetLocalHost();
        uint16 port = AssetCache::ASSET_SERVER_PORT;
        uint64 timeoutms = 60 * 1000;

        REGISTER_PREFERENCES(ConnectionParams)

        INTROSPECTION(ConnectionParams,
                      MEMBER(ip, "Asset cache/Asset Cache IP", DAVA::I_PREFERENCE)
                      MEMBER(port, "Asset cache/Asset Cache Port", DAVA::I_PREFERENCE)
                      MEMBER(timeoutms, "Asset cache/Asset Cache Timeout (ms)", DAVA::I_PREFERENCE)
                      )
    };

    AssetCacheClient(bool emulateNetworkLoop);
    ~AssetCacheClient() override;

    AssetCache::Error ConnectSynchronously(const ConnectionParams& connectionParams);
    void Disconnect();

    AssetCache::Error AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value);
    AssetCache::Error RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value);
    AssetCache::Error RemoveFromCacheSynchronously(const AssetCache::CacheItemKey& key);
    AssetCache::Error ClearCacheSynchronously();

    bool IsConnected() const;

private:
    void ProcessNetwork();

    AssetCache::Error WaitRequest();

    AssetCache::Error CheckStatusSynchronously();

    //ClientNetProxyListener
    void OnAddedToCache(const AssetCache::CacheItemKey& key, bool added) override;
    void OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value) override;
    void OnRemovedFromCache(const AssetCache::CacheItemKey& key, bool removed) override;
    void OnCacheCleared(bool cleared) override;
    void OnServerStatusReceived();
    void OnIncorrectPacketReceived(AssetCache::IncorrectPacketType) override;
    void OnClientProxyStateChanged() override;

private:
    struct Request
    {
        Request() = default;
        Request(AssetCache::ePacketID requestID_)
            : requestID(requestID_)
        {
        }
        Request(AssetCache::ePacketID requestID_, const AssetCache::CacheItemKey& key_, AssetCache::CachedItemValue* value_ = nullptr)
            : requestID(requestID_)
            , key(key_)
            , value(value_)
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
