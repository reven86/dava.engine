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

#include "AssetCache/AssetCache.h"
#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"

#include "AssetCache/AssetCacheClient.h"

using namespace DAVA;

namespace CacheRequestInternal
{
AssetCacheClient::ConnectionParams GetConnectionParams(ProgramOptions options)
{
    AssetCacheClient::ConnectionParams params;
    params.ip = options.GetOption("-ip").AsString();
    params.port = static_cast<uint16>(options.GetOption("-p").AsUInt32());
    params.timeoutms = options.GetOption("-t").AsUInt64() * 1000; // convert to ms

    return params;
}
}

CacheRequest::CacheRequest(const String& commandLineOptionName)
    : options(commandLineOptionName)
{
    options.AddOption("-ip", VariantType(AssetCache::GetLocalHost()), "Set ip adress of Asset Cache Server.");
    options.AddOption("-p", VariantType(static_cast<uint32>(AssetCache::ASSET_SERVER_PORT)), "Set port of Asset Cache Server.");
    options.AddOption("-h", VariantType(String("")), "Hash string of requested data");
    options.AddOption("-v", VariantType(false), "Verbose output.");
    options.AddOption("-t", VariantType(static_cast<uint64>(1)), "Connection timeout seconds.");
}

AssetCache::Error CacheRequest::Process(AssetCacheClient& cacheClient)
{
    if (options.GetOption("-v").AsBool())
    {
        Logger::Instance()->SetLogLevel(Logger::LEVEL_FRAMEWORK);
    }

    AssetCache::Error exitCode = cacheClient.ConnectSynchronously(CacheRequestInternal::GetConnectionParams(options));
    if (AssetCache::Error::NO_ERRORS == exitCode)
    {
        exitCode = SendRequest(cacheClient);
    }
    cacheClient.Disconnect();

    return exitCode;
}

AssetCache::Error CacheRequest::CheckOptions() const
{
    const String hash = options.GetOption("-h").AsString();
    if (hash.length() != AssetCache::HASH_SIZE * 2)
    {
        Logger::Error("[CacheRequest::%s] Wrong hash argument (%s)", __FUNCTION__, hash.c_str());
        return AssetCache::Error::WRONG_COMMAND_LINE;
    }

    return CheckOptionsInternal();
}
