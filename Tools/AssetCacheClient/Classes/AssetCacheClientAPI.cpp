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

#include "AssetCacheClientAPI.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"

using namespace DAVA;

AssetCacheClientAPI::AssetCacheClientAPI()
{
    client.AddListener(this);
}


AssetCacheClientConstants::ExitCodes AssetCacheClientAPI::ConnectBlocked(const DAVA::String &ip, DAVA::uint16 port, DAVA::uint64 timeoutms_)
{
    timeoutms = timeoutms_;

    bool connectCalled = client.Connect(ip, port);
    if (!connectCalled)
    {
        return AssetCacheClientConstants::EXIT_ADDRESS_RESOLVER_FAILED;
    }

    DAVA::uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (client.ChannelIsOpened() == false)
    {
        Net::NetCore::Instance()->Poll();

        DAVA::uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (client.ChannelIsOpened() == false))
        {
            Logger::Error("[AssetCacheClientAPI::%s] connection to %s:%hu refused by timeout (%lld ms)", __FUNCTION__, ip.c_str(), port, timeoutms);
            return AssetCacheClientConstants::EXIT_TIMEOUT;
        }
    }

    return AssetCacheClientConstants::EXIT_OK;
}

void AssetCacheClientAPI::Disconnect()
{
    client.Disconnect();
}

AssetCacheClientConstants::ExitCodes AssetCacheClientAPI::AddToCacheBlocked(const DAVA::AssetCache::CacheItemKey& key, const DAVA::AssetCache::CachedItemValue& value)
{
    bool requestSent = client.AddToCache(key, value);
    if (requestSent)
    {
        return WaitRequest();
    }

    return AssetCacheClientConstants::EXIT_CANNOT_SEND_REQUEST_ADD;
}

AssetCacheClientConstants::ExitCodes AssetCacheClientAPI::RequestFromCacheBlocked(const DAVA::AssetCache::CacheItemKey& key)
{
    bool requestSent = client.RequestFromCache(key);
    if (requestSent)
    {
        return WaitRequest();
    }

    return AssetCacheClientConstants::EXIT_CANNOT_SEND_REQUEST_GET;
}


AssetCacheClientConstants::ExitCodes AssetCacheClientAPI::WaitRequest()
{
    DAVA::uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (requestResult.recieved == false)
    {
        Net::NetCore::Instance()->Poll();

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((timeoutms > 0) && (deltaTime > timeoutms)) && (requestResult.recieved == false))
        {
            Logger::Error("[AssetCacheClientAPI::%s] Sending files refused by timeout (%lld sec)", __FUNCTION__, timeoutms);
            return AssetCacheClientConstants::EXIT_TIMEOUT;
        }
    }

    if (requestResult.succeed == false)
    {
        Logger::Error("[AssetCacheClientAPI::%s] Request (%d) failed by server", __FUNCTION__, requestResult.requestID);
        return AssetCacheClientConstants::EXIT_SERVER_ERROR;
    }

    return AssetCacheClientConstants::EXIT_OK;
}


void AssetCacheClientAPI::OnAddedToCache(const DAVA::AssetCache::CacheItemKey& key, bool added)
{
    if (requestResult.requestID == DAVA::AssetCache::PACKET_ADD_REQUEST)
    {
        requestResult.recieved = true;
        requestResult.succeed = added;
    }
    else
    {
        DAVA::Logger::Error("[AssetCacheClientAPI::%s] Wrong answer. Waiting answer on %d", __FUNCTION__, requestResult.requestID);
    }
}

void AssetCacheClientAPI::OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::AssetCache::CachedItemValue & value)
{
    if (requestResult.requestID == DAVA::AssetCache::PACKET_GET_REQUEST)
    {
        requestResult.recieved = true;
        requestResult.succeed = (value.IsEmpty() == false);
    }
    else
    {
        DAVA::Logger::Error("[AssetCacheClientAPI::%s] Wrong answer. Waiting answer on %d", __FUNCTION__, requestResult.requestID);
    }
}

