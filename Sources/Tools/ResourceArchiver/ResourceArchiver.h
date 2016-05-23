#pragma once

#include <FileSystem/FilePath.h>
#include <FileSystem/ResourceArchive.h>

namespace DAVA
{
class AssetCacheClient;

namespace ResourceArchiver
{
struct Params
{
    Vector<String> sourcesList;
    bool addHiddenFiles = false;
    DAVA::Compressor::Type compressionType = Compressor::Type::Lz4HC;
    FilePath archivePath;
    FilePath logPath;
    AssetCacheClient* assetCacheClient = nullptr;
};

bool CreateArchive(const Params& params);

} // namespace Archive
} // namespace DAVA
