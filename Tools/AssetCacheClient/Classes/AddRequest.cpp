#include "AddRequest.h"

#include "FileSystem/File.h"
#include "Platform/DateTime.h"
#include "Platform/DeviceInfo.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

#include "AssetCache/AssetCacheClient.h"

using namespace DAVA;

AddRequest::AddRequest()
    : CacheRequest("add")
{
    options.AddOption("-k", VariantType(String("")), "Key (hash string) of requested data");
    options.AddOption("-f", VariantType(String("")), "Files list to send files to server", true);
}

DAVA::AssetCache::Error AddRequest::SendRequest(AssetCacheClient& cacheClient)
{
    AssetCache::CacheItemKey key;
    key.FromString(options.GetOption("-k").AsString());

    AssetCache::CachedItemValue value;

    uint32 filesCount = options.GetOptionValuesCount("-f");
    for (uint32 i = 0; i < filesCount; ++i)
    {
        const FilePath path = options.GetOption("-f", i).AsString();
        ScopedPtr<File> file(File::Create(path, File::OPEN | File::READ));
        if (file)
        {
            std::shared_ptr<Vector<uint8>> data = std::make_shared<Vector<uint8>>();

            auto dataSize = file->GetSize();
            data.get()->resize(dataSize);

            auto read = file->Read(data.get()->data(), dataSize);
            DVASSERT(read == dataSize);

            value.Add(path.GetFilename(), data);
        }
        else
        {
            Logger::Error("[AddRequest::%s] Cannot read file(%s)", __FUNCTION__, path.GetStringValue().c_str());
            return AssetCache::Error::READ_FILES;
        }
    }

    AssetCache::CachedItemValue::Description description;
    description.machineName = WStringToString(DeviceInfo::GetName());

    DateTime timeNow = DateTime::Now();
    description.creationDate = WStringToString(timeNow.GetLocalizedDate()) + "_" + WStringToString(timeNow.GetLocalizedTime());
    description.comment = "Asset Cache Client";

    value.SetDescription(description);
    value.UpdateValidationData();
    return cacheClient.AddToCacheSynchronously(key, value);
}

DAVA::AssetCache::Error AddRequest::CheckOptionsInternal() const
{
    const String hash = options.GetOption("-k").AsString();
    if (hash.length() != AssetCache::HASH_SIZE * 2)
    {
        Logger::Error("[CacheRequest::%s] Wrong hash argument (%s)", __FUNCTION__, hash.c_str());
        return AssetCache::Error::WRONG_COMMAND_LINE;
    }

    const String filepath = options.GetOption("-f").AsString();
    if (filepath.empty())
    {
        Logger::Error("[AddRequest::%s] Empty file list", __FUNCTION__);
        return AssetCache::Error::WRONG_COMMAND_LINE;
    }

    return AssetCache::Error::NO_ERRORS;
}
