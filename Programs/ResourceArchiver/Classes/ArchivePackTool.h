#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/ResourceArchive.h"
#include "CommandLineTool.h"
#include <Tools/AssetCache/AssetCacheClient.h>

class ArchivePackTool : public CommandLineTool
{
public:
    ArchivePackTool();

private:
    enum class Source
    {
        UseListFiles,
        UseSrc,
        UseMetaDB,
        Unknown
    };

    bool ConvertOptionsToParamsInternal() override;
    int ProcessInternal() override;

    void CollectAllFilesInDirectory(const DAVA::String& pathDirName, DAVA::Vector<DAVA::String>& output);

    DAVA::String compressionStr;
    DAVA::Compressor::Type compressionType;
    bool addHidden = false;
    bool useCache = false;
    bool genDvpl = false;
    DAVA::AssetCacheClient::ConnectionParams assetCacheParams;
    DAVA::String logFileName;
    DAVA::String srcDir;
    DAVA::List<DAVA::String> listFiles;
    DAVA::Vector<DAVA::String> srcFiles;
    DAVA::String packFileName;
    DAVA::String baseDir;
    DAVA::String metaDbPath;
    Source source = Source::Unknown;
};
