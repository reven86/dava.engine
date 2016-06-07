#include "ServerLogics.h"

#include "Concurrency/LockGuard.h"

ServerLogics::RequestDescription::RequestDescription(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& _key, DAVA::AssetCache::ePacketID _request)
    : clientChannel(channel)
    , key(_key)
    , request(_request)
{
}

ServerLogics::ServerTask::ServerTask(const DAVA::AssetCache::CacheItemKey& _key, DAVA::AssetCache::ePacketID _request)
    : key(_key)
    , request(_request)
{
}

ServerLogics::ServerTask::ServerTask(const DAVA::AssetCache::CacheItemKey& _key, DAVA::AssetCache::CachedItemValue&& _value, DAVA::AssetCache::ePacketID _request)
    : key(_key)
    , value(std::move(_value))
    , request(_request)
{
}

void ServerLogics::Init(DAVA::AssetCache::ServerNetProxy* server_, const DAVA::String& serverName_, DAVA::AssetCache::ClientNetProxy* client_, DAVA::AssetCache::CacheDB* dataBase_)
{
    server = server_;
    serverName = serverName_;

    client = client_;

    dataBase = dataBase_;
}

void ServerLogics::OnAddToCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key, DAVA::AssetCache::CachedItemValue&& value)
{
    if ((nullptr != server) && (nullptr != channel))
    {
        DAVA::AssetCache::CachedItemValue::Description description = value.GetDescription();
        description.addingChain += "/" + serverName;
        value.SetDescription(description);

        bool isValid = value.IsValid();
        if (isValid && value.GetSize() > dataBase->GetStorageSize())
        {
            isValid = false;
            Logger::Warning("[%s] Inserted size %u is bigger than max storage size %u", __FUNCTION__, value.GetSize(), dataBase->GetStorageSize());
        }

        server->AddedToCache(channel, key, isValid);

        if (isValid)
        {
            dataBase->Insert(key, value);

            //add task for lazy sending of files;
            serverTasks.emplace_back(key, std::forward<DAVA::AssetCache::CachedItemValue>(value), DAVA::AssetCache::PACKET_ADD_REQUEST);
        }
        else
        {
            DAVA::Logger::Error("[%s] Received invalid data from %s at %s", __FUNCTION__, description.machineName.c_str(), description.creationDate.c_str());
        }
    }
}

void ServerLogics::OnRequestedFromCache(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key)
{
    if ((nullptr != server) && (nullptr != dataBase) && (nullptr != channel))
    {
        auto entry = dataBase->Get(key);
        if (nullptr != entry)
        { // Found in db.
            DAVA::AssetCache::CachedItemValue value = entry->GetValue();
            DAVA::AssetCache::CachedItemValue::Description description = value.GetDescription();
            description.receivingChain += "/" + serverName;
            value.SetDescription(description);

            server->Send(channel, key, value);

            { //add task for lazy sending of files;
                serverTasks.emplace_back(key, DAVA::AssetCache::PACKET_WARMING_UP_REQUEST);
            }
        }
        else if (client->RequestFromCache(key))
        { // Not found in db. Ask from remote cache.
            waitedRequests.emplace_back(channel, key, DAVA::AssetCache::PACKET_GET_REQUEST);
        }
        else
        { // Not found in db. Remote server isn't connected.
            server->Send(channel, key, DAVA::AssetCache::CachedItemValue());
        }
    }
}

void ServerLogics::OnWarmingUp(DAVA::Net::IChannel* channel, const DAVA::AssetCache::CacheItemKey& key)
{
    if (nullptr != dataBase)
    {
        dataBase->UpdateAccessTimestamp(key);
    }
}

void ServerLogics::OnChannelClosed(DAVA::Net::IChannel* channel, const DAVA::char8*)
{
    if (waitedRequests.size())
    {
        auto iter = std::find_if(waitedRequests.begin(), waitedRequests.end(), [&channel](const RequestDescription& description) -> bool
                                 {
                                     return (description.clientChannel == channel);
                                 });

        if (iter != waitedRequests.end())
        {
            waitedRequests.erase(iter);
        }
    }
}

void ServerLogics::OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::AssetCache::CachedItemValue& value)
{
    if (nullptr != dataBase && !value.IsEmpty() && value.IsValid())
    {
        dataBase->Insert(key, value);
    }

    if ((nullptr != server) && waitedRequests.size())
    {
        auto iter = std::find_if(waitedRequests.begin(), waitedRequests.end(), [&key](const RequestDescription& description) -> bool
                                 {
                                     return (description.key == key) && (description.request == DAVA::AssetCache::PACKET_GET_REQUEST);
                                 });

        if (iter != waitedRequests.end())
        {
            RequestDescription& description = (*iter);
            if (nullptr != description.clientChannel)
            {
                DAVA::AssetCache::CachedItemValue sendValue = value;
                DAVA::AssetCache::CachedItemValue::Description descr = sendValue.GetDescription();
                descr.receivingChain += "/" + serverName;
                sendValue.SetDescription(descr);
                server->Send(description.clientChannel, key, sendValue);
            }
            waitedRequests.erase(iter);
        }
        else
        {
            DAVA::Logger::Warning("[ServerLogics::%s] Connection was closed by client", __FUNCTION__);
        }
    }
}

void ServerLogics::ProcessServerTasks()
{
    if (!serverTasks.empty() && client && client->ChannelIsOpened())
    {
        for (const auto& task : serverTasks)
        {
            switch (task.request)
            {
            case DAVA::AssetCache::PACKET_ADD_REQUEST:
                client->AddToCache(task.key, task.value);
                break;

            case DAVA::AssetCache::PACKET_WARMING_UP_REQUEST:
                client->WarmingUp(task.key);
                break;

            default:
                break;
            }
        }

        serverTasks.clear();
    }
}

void ServerLogics::Update()
{
    ProcessServerTasks();

    if (dataBase)
    {
        dataBase->Update();
    }
}
