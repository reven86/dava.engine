#include "RemoveRequest.h"

#include "FileSystem/File.h"
#include "Platform/DateTime.h"
#include "Platform/DeviceInfo.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

#include "AssetCache/AssetCacheClient.h"

RemoveRequest::RemoveRequest()
    : CacheRequest("remove")
{
    options.AddOption("-k", DAVA::VariantType(DAVA::String("")), "Key (hash string) of requested data");
}

DAVA::AssetCache::Error RemoveRequest::SendRequest(DAVA::AssetCacheClient& cacheClient)
{
    DAVA::AssetCache::CacheItemKey key;
    key.FromString(options.GetOption("-k").AsString());
    return cacheClient.RemoveFromCacheSynchronously(key);
}

DAVA::AssetCache::Error RemoveRequest::CheckOptionsInternal() const
{
    const DAVA::String hash = options.GetOption("-k").AsString();
    if (hash.length() != DAVA::AssetCache::HASH_SIZE * 2)
    {
        DAVA::Logger::Error("[CacheRequest::%s] Wrong hash argument (%s)", __FUNCTION__, hash.c_str());
        return DAVA::AssetCache::Error::WRONG_COMMAND_LINE;
    }

    return DAVA::AssetCache::Error::NO_ERRORS;
}
