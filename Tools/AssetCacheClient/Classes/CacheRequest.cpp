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
