#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/ResourceArchive.h"
#include "CommandLineTool.h"
#include "AssetCache/CacheItemKey.h"
#include "AssetCache/AssetCacheClient.h"

class ArchivePackTool : public CommandLineTool
{
public:
    ArchivePackTool();

private:
    enum class Source
    {
        UseListFiles,
        UseSrc,
        Unknown
    };

    bool ConvertOptionsToParamsInternal() override;
    int ProcessInternal() override;

    void CollectAllFilesInDirectory(const DAVA::String& pathDirName, DAVA::Vector<DAVA::String>& output);

    void ConstructCacheKey(DAVA::AssetCache::CacheItemKey& key, const DAVA::Vector<DAVA::String>& files, const DAVA::String& compression) const;
    bool RetrieveFromCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::FilePath& pathToPack, const DAVA::FilePath& pathToLog) const;
    bool AddToCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::FilePath& pathToPack, const DAVA::FilePath& pathToLog) const;

    DAVA::String compressionStr;
    DAVA::Compressor::Type compressionType;
    bool addHidden = false;
    bool useCache = false;
    DAVA::AssetCacheClient::ConnectionParams assetCacheParams;
    DAVA::String logFileName;
    DAVA::String srcDir;
    DAVA::List<DAVA::String> listFiles;
    DAVA::Vector<DAVA::String> srcFiles;
    DAVA::String packFileName;
    Source source = Source::Unknown;
};

