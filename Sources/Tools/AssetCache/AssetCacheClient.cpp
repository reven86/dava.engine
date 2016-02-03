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
#include "Concurrency/Thread.h"
#include "Job/JobManager.h"

using namespace DAVA;

AssetCacheClient::AssetCacheClient()
{
    DVASSERT(JobManager::Instance() != nullptr);

    client.AddListener(this);
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
    JobManager::Instance()->CreateWorkerJob(MakeFunction(this, &AssetCacheClient::ProcessNetwork));

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
}

AssetCache::ErrorCodes AssetCacheClient::AddToCacheBlocked(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    requestResult.recieved = false;
    requestResult.succeed = false;
    requestResult.requestID = AssetCache::PACKET_ADD_REQUEST;

    bool requestSent = client.AddToCache(key, value);
    if (requestSent)
    {
        return WaitRequest();
    }

    return AssetCache::ERROR_CANNOT_SEND_REQUEST_ADD;
}

AssetCache::ErrorCodes AssetCacheClient::RequestFromCacheBlocked(const AssetCache::CacheItemKey& key, const FilePath& outputFolder)
{
    requestResult.recieved = false;
    requestResult.succeed = false;
    requestResult.requestID = AssetCache::PACKET_GET_REQUEST;

    DVASSERT(requests.count(key) == 0);
    requests[key] = outputFolder;

    bool requestSent = client.RequestFromCache(key);
    if (requestSent)
    {
        return WaitRequest();
    }
    else
    {
        requests.erase(key);
    }

    return AssetCache::ERROR_CANNOT_SEND_REQUEST_GET;
}

AssetCache::ErrorCodes AssetCacheClient::WaitRequest()
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (requestResult.recieved == false)
    {
        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (requestResult.recieved == false))
        {
            Logger::Error("[AssetCacheClient::%s] Sending files refused by timeout (%lld ms)", __FUNCTION__, timeoutms);
            return AssetCache::ERROR_OPERATION_TIMEOUT;
        }
    }

    if (requestResult.succeed == false)
    {
        Logger::Error("[AssetCacheClient::%s] Request (%d) failed by server", __FUNCTION__, requestResult.requestID);
        return AssetCache::ERROR_SERVER_ERROR;
    }

    return AssetCache::ERROR_OK;
}

void AssetCacheClient::OnAddedToCache(const AssetCache::CacheItemKey& key, bool added)
{
    if (requestResult.requestID == AssetCache::PACKET_ADD_REQUEST)
    {
        requestResult.succeed = added;
        requestResult.recieved = true;
    }
    else
    {
        Logger::Error("[AssetCacheClient::%s] Wrong answer. Waiting answer on %d", __FUNCTION__, requestResult.requestID);
    }
}

void AssetCacheClient::OnReceivedFromCache(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    if (requestResult.requestID == AssetCache::PACKET_GET_REQUEST)
    {
        requestResult.succeed = (value.IsEmpty() == false);
        requestResult.recieved = true;
        if (requestResult.succeed)
        {
            const FilePath& outputFolder = requests[key];

            FileSystem::Instance()->CreateDirectory(outputFolder, true);
            value.Export(outputFolder);
        }
    }
    else
    {
        Logger::Error("[AssetCacheClient::%s] Wrong answer. Waiting answer on %d", __FUNCTION__, requestResult.requestID);
    }

    requests.erase(key);
}

void AssetCacheClient::AddListener(AssetCache::ClientNetProxyListener* listener)
{
    client.AddListener(listener);
}

void AssetCacheClient::RemoveListener(AssetCache::ClientNetProxyListener* listener)
{
    client.RemoveListener(listener);
}

void AssetCacheClient::ProcessNetwork()
{
    while (isActive)
    {
        Net::NetCore::Instance()->Poll();
    }
}

void AssetCacheClient::OnAssetClientStateChanged()
{
    if (client.ChannelIsOpened() == false)
    {
        isActive = false;
    }
}
