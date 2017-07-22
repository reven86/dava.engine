#pragma once

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
        ConnectionParams();
        ~ConnectionParams();
        String ip = AssetCache::GetLocalHost();
        uint16 port = AssetCache::ASSET_SERVER_PORT;
        uint64 timeoutms = 60u * 1000u;

        INTROSPECTION(ConnectionParams,
                      MEMBER(ip, "Asset cache/Asset Cache IP", DAVA::I_PREFERENCE)
                      MEMBER(port, "Asset cache/Asset Cache Port", DAVA::I_PREFERENCE)
                      MEMBER(timeoutms, "Asset cache/Asset Cache Timeout (ms)", DAVA::I_PREFERENCE)
                      )
    };

    AssetCacheClient();
    ~AssetCacheClient() override;

    AssetCache::Error ConnectSynchronously(const ConnectionParams& connectionParams);
    void Disconnect();

    AssetCache::Error AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value);
    AssetCache::Error RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value);
    AssetCache::Error RemoveFromCacheSynchronously(const AssetCache::CacheItemKey& key);
    AssetCache::Error ClearCacheSynchronously();

    uint64 GetTimeoutMs() const;
    bool IsConnected() const;

private:
    AssetCache::Error WaitRequest(uint64 requestTimeoutMs);
    AssetCache::Error CheckStatusSynchronously();
    void ProcessNetwork();

    //ClientNetProxyListener
    void OnAddedToCache(const AssetCache::CacheItemKey& key, bool added) override;
    void OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value) override;
    void OnRemovedFromCache(const AssetCache::CacheItemKey& key, bool removed) override;
    void OnCacheCleared(bool cleared) override;
    void OnServerStatusReceived() override;
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
            : key(key_)
            , value(value_)
            , requestID(requestID_)
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

    Dispatcher<Function<void()>> dispatcher;
    AssetCache::ClientNetProxy client;

    uint64 lightRequestTimeoutMs = 60u * 1000u;
    uint64 heavyRequestTimeoutMs = 60u * 1000u;
    uint64 currentTimeoutMs = 60u * 1000u;

    Mutex requestLocker;
    Mutex connectEstablishLocker;
    Request request;

    std::atomic<bool> isActive;
};

inline uint64 AssetCacheClient::GetTimeoutMs() const
{
    return currentTimeoutMs;
}

} //END of DAVA
