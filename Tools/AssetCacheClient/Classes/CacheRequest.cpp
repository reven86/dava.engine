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

#include "CacheRequest.h"
#include "Constants.h"

#include "AssetCache/AssetCache.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"

using namespace DAVA;

CacheRequest::CacheRequest(const String& commandLineOptionName)
    : options(commandLineOptionName)
{
    options.AddOption("-ip", VariantType(String("127.0.0.1")), "Set ip adress of Asset Cache Server.");
    options.AddOption("-p", VariantType(static_cast<uint32>(AssetCache::ASSET_SERVER_PORT)), "Set port of Asset Cache Server.");
    options.AddOption("-h", VariantType(String("")), "Hash string of requested data");
    options.AddOption("-v", VariantType(false), "Verbose output.");
    options.AddOption("-t", VariantType(static_cast<uint64>(1)), "Connection timeout seconds.");

    client.AddListener(this);
}

int CacheRequest::Process()
{
    if (options.GetOption("-v").AsBool())
    {
        Logger::Instance()->SetLogLevel(Logger::LEVEL_FRAMEWORK);
    }

    int exitCode = Connect();
    if (AssetCacheClientConstants::EXIT_OK != exitCode)
    {
        return exitCode;
    }

    exitCode = SendRequest();
    if (exitCode == AssetCacheClientConstants::EXIT_OK)
    {
        exitCode = WaitRequest();
    }

    Disconnect();

    return exitCode;
}

int CacheRequest::CheckOptions() const
{
    const String hash = options.GetOption("-h").AsString();
    if (hash.length() != AssetCache::HASH_SIZE * 2)
    {
        Logger::Error("[CacheRequest::%s] Wrong hash argument (%s)", __FUNCTION__, hash.c_str());
        return AssetCacheClientConstants::EXIT_WRONG_COMMAND_LINE;
    }

    return CheckOptionsInternal();
}

int CacheRequest::Connect()
{
    const String ipAdress = options.GetOption("-ip").AsString();
    const uint16 port = static_cast<uint16>(options.GetOption("-p").AsUInt32());

    client.Connect(ipAdress, port);

    const auto startTime = SystemTimer::Instance()->AbsoluteMS();
    const auto connectionTimeout = options.GetOption("-t").AsUInt64() * 1000; // convert to ms

    while (client.ChannelIsOpened() == false)
    {
        Net::NetCore::Instance()->Poll();

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((connectionTimeout > 0) && (deltaTime > connectionTimeout)) && (client.ChannelIsOpened() == false))
        {
            Logger::Error("[CacheRequest::%s] connection to %s refused by timeout (%lld sec)", __FUNCTION__, ipAdress.c_str(), connectionTimeout / 1000);
            return AssetCacheClientConstants::EXIT_TIMEOUT;
        }
    }

    return AssetCacheClientConstants::EXIT_OK;
}

int CacheRequest::WaitRequest()
{
    const auto startTime = SystemTimer::Instance()->AbsoluteMS();
    const auto connectionTimeout = options.GetOption("-t").AsUInt64() * 1000; // convert to ms

    while (requestResult.recieved == false)
    {
        Net::NetCore::Instance()->Poll();

        auto deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (((connectionTimeout > 0) && (deltaTime > connectionTimeout)) && (requestResult.recieved == false))
        {
            Logger::Error("[CacheRequest::%s] Sending files refused by timeout (%lld sec)", __FUNCTION__, (connectionTimeout / 1000));
            return AssetCacheClientConstants::EXIT_TIMEOUT;
        }
    }

    if (requestResult.succeed == false)
    {
        Logger::Error("[CacheRequest::%s] Request failed by server", __FUNCTION__);
        return AssetCacheClientConstants::EXIT_SERVER_ERROR;
    }

    return AssetCacheClientConstants::EXIT_OK;
}

int CacheRequest::Disconnect()
{
    client.Disconnect();
    return AssetCacheClientConstants::EXIT_OK;
}