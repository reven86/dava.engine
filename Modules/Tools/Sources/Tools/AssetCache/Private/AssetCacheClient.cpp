#include "Tools/AssetCache/AssetCacheClient.h"

#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"
#include "Preferences/PreferencesRegistrator.h"
#include "Time/SystemTimer.h"
#include "Utils/StringFormat.h"
#include "Logger/Logger.h"
#include "Network/NetCore.h"

namespace DAVA
{
namespace AssetCacheClientDetail
{
InspInfoRegistrator inspInfoRegistrator(AssetCacheClient::ConnectionParams::TypeInfo(), {
                                                                                        PREF_ARG("ip", DAVA::AssetCache::GetLocalHost()),
                                                                                        PREF_ARG("port", DAVA::AssetCache::ASSET_SERVER_PORT),
                                                                                        PREF_ARG("timeoutms", DAVA::uint64(10 * 1000))
                                                                                        });
};

AssetCacheClient::AssetCacheClient()
    : dispatcher([](const Function<void()>& fn) { fn(); })
    , client(&dispatcher)
    , isActive(false)
{
    dispatcher.LinkToCurrentThread();
    client.AddListener(this);
}

AssetCacheClient::~AssetCacheClient()
{
    client.RemoveListener(this);

    DVASSERT(isActive == false);
}

AssetCache::Error AssetCacheClient::ConnectSynchronously(const ConnectionParams& connectionParams)
{
    isActive = true;
    lightRequestTimeoutMs = connectionParams.timeoutms;
    heavyRequestTimeoutMs = lightRequestTimeoutMs + (100u * 1000u);
    currentTimeoutMs = lightRequestTimeoutMs;

    client.Connect(connectionParams.ip, AssetCache::ASSET_SERVER_PORT);

    {
        LockGuard<Mutex> guard(connectEstablishLocker);

        uint64 startTime = SystemTimer::GetMs();
        while (client.ChannelIsOpened() == false)
        {
            ProcessNetwork();
            if (!isActive)
            {
                return AssetCache::Error::CANNOT_CONNECT;
            }

            uint64 deltaTime = SystemTimer::GetMs() - startTime;
            if (((currentTimeoutMs > 0) && (deltaTime > currentTimeoutMs)) && (client.ChannelIsOpened() == false))
            {
                Logger::Error("Timeout on connecting to asset cache %s (%lld ms)", connectionParams.ip.c_str(), currentTimeoutMs);
                isActive = false;
                return AssetCache::Error::OPERATION_TIMEOUT;
            }
        }
    }

    return CheckStatusSynchronously();
}

void AssetCacheClient::Disconnect()
{
    isActive = false;

    { // wait for connection establishing loop is finished
        LockGuard<Mutex> guard(connectEstablishLocker);
    }

    client.Disconnect();
}

AssetCache::Error AssetCacheClient::CheckStatusSynchronously()
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request();
        request.requestID = AssetCache::ePacketID::PACKET_STATUS_REQUEST;
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestServerStatus();
    if (requestSent)
    {
        resultCode = WaitRequest(lightRequestTimeoutMs);
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_ADD_REQUEST, key);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestAddData(key, value);
    if (requestSent)
    {
        resultCode = WaitRequest(heavyRequestTimeoutMs);
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value)
{
    DVASSERT(value != nullptr);

    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_GET_REQUEST, key, value);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestData(key);
    if (requestSent)
    {
        resultCode = WaitRequest(heavyRequestTimeoutMs);
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::RemoveFromCacheSynchronously(const AssetCache::CacheItemKey& key)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_REMOVE_REQUEST, key);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestRemoveData(key);
    if (requestSent)
    {
        resultCode = WaitRequest(lightRequestTimeoutMs);
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::ClearCacheSynchronously()
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_CLEAR_REQUEST);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestClearCache();
    if (requestSent)
    {
        resultCode = WaitRequest(lightRequestTimeoutMs);
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::WaitRequest(uint64 timeoutMs)
{
    currentTimeoutMs = timeoutMs;

    uint64 startTime = SystemTimer::GetMs();

    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    while (currentRequest.recieved == false)
    {
        ProcessNetwork();

        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }

        uint64 deltaTime = SystemTimer::GetMs() - startTime;
        if (((timeoutMs > 0) && (deltaTime > timeoutMs)) && (currentRequest.recieved == false) && (currentRequest.processingRequest == false))
        {
            Logger::Debug("Operation timeout: (%lld ms)", timeoutMs);
            return AssetCache::Error::OPERATION_TIMEOUT;
        }
    }

    if (currentRequest.result == AssetCache::Error::NO_ERRORS)
    {
        while (currentRequest.processingRequest)
        {
            ProcessNetwork();
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }
    }

    return currentRequest.result;
}

void AssetCacheClient::OnServerStatusReceived()
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_STATUS_REQUEST)
    {
        request.result = AssetCache::Error::NO_ERRORS;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnAddedToCache(const AssetCache::CacheItemKey& key, bool added)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_ADD_REQUEST && request.key == key)
    {
        request.result = (added) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    if (currentRequest.requestID == AssetCache::PACKET_GET_REQUEST && currentRequest.key == key)
    {
        auto DumpInfo = [](const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
        {
            const AssetCache::CachedItemValue::Description& description = value.GetDescription();
            Logger::Info("Data got from cache. Generated %s on mashine %s (%s)",
                         description.creationDate.c_str(),
                         description.machineName.c_str(),
                         description.comment.c_str());
            Logger::Info("addingChain: %s, receivingChain: %s", description.addingChain.c_str(), description.receivingChain.c_str());
        };

        if (value.IsEmpty())
        {
            LockGuard<Mutex> guard(requestLocker);
            request.result = AssetCache::Error::NOT_FOUND_ON_SERVER;
            request.recieved = true;
            request.processingRequest = false;
        }
        else if (value.IsValid() == false)
        {
            LockGuard<Mutex> guard(requestLocker);
            request.result = AssetCache::Error::CORRUPTED_DATA;
            request.recieved = true;
            request.processingRequest = false;

            DumpInfo(key, value);
        }
        else
        {
            { // mark request as recieved and processed
                LockGuard<Mutex> guard(requestLocker);
                request.result = AssetCache::Error::NO_ERRORS;
                request.recieved = true;
                request.processingRequest = true;

                DVASSERT(request.value != nullptr, "Request object that waits for response of data, should have valid pointer to AssetCacheValue");
                *(request.value) = value;

                DumpInfo(key, value);
                request.processingRequest = false;
            }
        }
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnRemovedFromCache(const AssetCache::CacheItemKey& key, bool removed)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_REMOVE_REQUEST && request.key == key)
    {
        request.result = (removed) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnCacheCleared(bool cleared)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_CLEAR_REQUEST)
    {
        request.result = (cleared) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnIncorrectPacketReceived(AssetCache::IncorrectPacketType type)
{
    LockGuard<Mutex> guard(requestLocker);
    request.recieved = true;
    request.processingRequest = false;

    switch (type)
    {
    case AssetCache::IncorrectPacketType::UNDEFINED_DATA:
        request.result = AssetCache::Error::CORRUPTED_DATA;
        break;
    case AssetCache::IncorrectPacketType::UNSUPPORTED_VERSION:
        request.result = AssetCache::Error::UNSUPPORTED_VERSION;
        break;
    case AssetCache::IncorrectPacketType::UNEXPECTED_PACKET:
        request.result = AssetCache::Error::UNEXPECTED_PACKET;
        break;
    default:
        DVASSERT(false, Format("Unexpected incorrect packet type: %d", type).c_str());
        request.result = AssetCache::Error::CORRUPTED_DATA;
        break;
    }
}

void AssetCacheClient::OnClientProxyStateChanged()
{
    if (client.ChannelIsOpened() == false)
    {
        isActive = false;

        LockGuard<Mutex> guard(requestLocker);
        request.recieved = true;
        request.processingRequest = false;
        request.result = AssetCache::Error::CANNOT_CONNECT;
    }
}

bool AssetCacheClient::IsConnected() const
{
    return client.ChannelIsOpened();
}

void AssetCacheClient::ProcessNetwork()
{
    Net::NetCore::Instance()->Update();
    if (dispatcher.HasEvents())
    {
        dispatcher.ProcessEvents();
    }
}

AssetCacheClient::ConnectionParams::ConnectionParams()
{
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

AssetCacheClient::ConnectionParams::~ConnectionParams()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

} //END of DAVA
