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

#include "AssetCacheClient.h"

#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Thread.h"
#include "Job/JobManager.h"

namespace DAVA
{
AssetCacheClient::AssetCacheClient(bool emulateNetworkLoop_)
    : isActive(false)
    , isJobStarted(false)
    , emulateNetworkLoop(emulateNetworkLoop_)
{
    DVASSERT(JobManager::Instance() != nullptr);

    client.AddListener(this);
}

AssetCacheClient::~AssetCacheClient()
{
    if (requests.empty() == false)
    {
        Logger::Error("[AssetCacheClient::~AssetCacheClient] %d requests were not received", requests.size());
    }
}

AssetCache::ErrorCodes AssetCacheClient::ConnectBlocked(const ConnectionParams& connectionParams)
{
    timeoutms = connectionParams.timeoutms;

    bool connectCalled = client.Connect(connectionParams.ip, connectionParams.port);
    if (!connectCalled)
    {
        return AssetCache::ERROR_ADDRESS_RESOLVER_FAILED;
    }

    isActive = true;
    if (emulateNetworkLoop)
    {
        JobManager::Instance()->CreateWorkerJob(MakeFunction(this, &AssetCacheClient::ProcessNetwork));
    }

    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (client.ChannelIsOpened() == false)
    {
        uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (client.ChannelIsOpened() == false))
        {
            Logger::Error("[AssetCacheClient::%s] connection to %s:%hu refused by timeout (%lld ms)", __FUNCTION__, connectionParams.ip.c_str(), connectionParams.port, connectionParams.timeoutms);
            isActive = false;
            return AssetCache::ERROR_OPERATION_TIMEOUT;
        }
    }

    return AssetCache::ERROR_OK;
}

void AssetCacheClient::Disconnect()
{
    isActive = false;
    client.Disconnect();

    while (isJobStarted)
    {
        //wait for finishing of networking
    }
}

AssetCache::ErrorCodes AssetCacheClient::AddToCacheBlocked(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        requestResult.recieved = false;
        requestResult.succeed = false;
        requestResult.processingRequest = false;
        requestResult.requestID = AssetCache::PACKET_ADD_REQUEST;
    }

    bool requestSent = client.AddToCache(key, value);
    if (requestSent)
    {
        return WaitRequest();
    }

    return AssetCache::ERROR_CANNOT_SEND_REQUEST_ADD;
}

AssetCache::ErrorCodes AssetCacheClient::RequestFromCacheBlocked(const AssetCache::CacheItemKey& key, const FilePath& outputFolder)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        requestResult.recieved = false;
        requestResult.succeed = false;
        requestResult.processingRequest = false;
        requestResult.requestID = AssetCache::PACKET_GET_REQUEST;

        DVASSERT(requests.count(key) == 0);
        requests[key] = outputFolder;
    }

    bool requestSent = client.RequestFromCache(key);
    if (requestSent)
    {
        return WaitRequest();
    }
    else
    {
        LockGuard<Mutex> guard(requestLocker);
        requests.erase(key);
    }

    return AssetCache::ERROR_CANNOT_SEND_REQUEST_GET;
}

AssetCache::ErrorCodes AssetCacheClient::WaitRequest()
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    ResultOfRequest currentRequestResult;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequestResult = requestResult;
    }

    while (currentRequestResult.recieved == false)
    {
        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequestResult = requestResult;
        }

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (currentRequestResult.recieved == false) && (currentRequestResult.processingRequest == false))
        {
            return AssetCache::ERROR_OPERATION_TIMEOUT;
        }
    }

    if (currentRequestResult.succeed == false)
    {
        return (currentRequestResult.requestID == AssetCache::PACKET_GET_REQUEST) ? AssetCache::ERROR_NOT_FOUND_ON_SERVER : AssetCache::ERROR_SERVER_ERROR;
    }

    while (currentRequestResult.processingRequest)
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequestResult = requestResult;
    }

    return AssetCache::ERROR_OK;
}

void AssetCacheClient::OnAddedToCache(const AssetCache::CacheItemKey& key, bool added)
{
    LockGuard<Mutex> guard(requestLocker);
    if (requestResult.requestID == AssetCache::PACKET_ADD_REQUEST)
    {
        requestResult.succeed = added;
        requestResult.recieved = true;
        requestResult.processingRequest = false;
    }
    else
    {
        Logger::Error("[AssetCacheClient::%s] Wrong answer. Waiting answer on %d", __FUNCTION__, requestResult.requestID);
    }
}

void AssetCacheClient::OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    FilePath outputFolder;
    ResultOfRequest currentRequestResult;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequestResult = requestResult;
        outputFolder = requests[key];
        requests.erase(key);
    }

    if (currentRequestResult.requestID == AssetCache::PACKET_GET_REQUEST)
    {
        const bool dataRetrived = (!value.IsEmpty()) && value.IsValid();

        {
            LockGuard<Mutex> guard(requestLocker);
            requestResult.succeed = dataRetrived;
            requestResult.recieved = true;
            requestResult.processingRequest = true;
        }

        if (dataRetrived)
        {
            const AssetCache::CachedItemValue::Description& description = value.GetDescription();
            Logger::Info("[AssetCacheClient::%s] machine(%s), date(%s), hash (%s)", __FUNCTION__, description.machineName.c_str(), description.creationDate.c_str(), AssetCache::KeyToString(key).c_str());
            Logger::FrameworkDebug("[AssetCacheClient::%s] serverPath(%s), clientPath(%s), comment(%s)", __FUNCTION__, description.serverPath.c_str(), description.clientPath.c_str(), description.comment.c_str());

            FileSystem::Instance()->CreateDirectory(outputFolder, true);
            value.Export(outputFolder);
        }
        else if (!value.IsValid())
        {
            Logger::Error("[AssetCacheClient::%s] Retrived Invalid value.", __FUNCTION__);
        }

        {
            LockGuard<Mutex> guard(requestLocker);
            requestResult.processingRequest = false;
        }
    }
    else
    {
        Logger::Error("[AssetCacheClient::%s] Wrong answer. Waiting answer on %d", __FUNCTION__, currentRequestResult.requestID);
    }
}


void AssetCacheClient::ProcessNetwork()
{
    isJobStarted = true;

    while (isActive)
    {
        Net::NetCore::Instance()->Poll();
    }

    isJobStarted = false;
}

void AssetCacheClient::OnAssetClientStateChanged()
{
    if (client.ChannelIsOpened() == false)
    {
        isActive = false;
    }
}

bool AssetCacheClient::IsConnected() const
{
    return client.ChannelIsOpened();
}

} //END of DAVA
