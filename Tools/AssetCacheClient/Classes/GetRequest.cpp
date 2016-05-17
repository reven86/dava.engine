#include "GetRequest.h"

#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"

#include "AssetCache/AssetCacheClient.h"

using namespace DAVA;

GetRequest::GetRequest()
    : CacheRequest("get")
{
    options.AddOption("-f", VariantType(String("")), "Folder to save files from server");
}

AssetCache::Error GetRequest::SendRequest(AssetCacheClient& cacheClient)
{
    AssetCache::CacheItemKey key;
    AssetCache::StringToKey(options.GetOption("-h").AsString(), key);

    FilePath folder = options.GetOption("-f").AsString();
    folder.MakeDirectoryPathname();

    return cacheClient.RequestFromCacheSynchronously(key, folder);
}

AssetCache::Error GetRequest::CheckOptionsInternal() const
{
    const String folderpath = options.GetOption("-f").AsString();
    if (folderpath.empty())
    {
        Logger::Error("[GetRequest::%s] Empty folderpath", __FUNCTION__);
        return AssetCache::Error::WRONG_COMMAND_LINE;
    }

    return AssetCache::Error::NO_ERRORS;
}
