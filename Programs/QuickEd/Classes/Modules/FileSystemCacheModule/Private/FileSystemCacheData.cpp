#include "Modules/FileSystemCacheModule/FileSystemCacheData.h"
#include <QtTools/ProjectInformation/FileSystemCache.h>

FileSystemCacheData::FileSystemCacheData(const QStringList& extensions)
    : fileSystemCache(new FileSystemCache(extensions))
{
}

FileSystemCacheData::~FileSystemCacheData() = default;

FileSystemCache* FileSystemCacheData::GetFileSystemCache()
{
    return fileSystemCache.get();
}

const FileSystemCache* FileSystemCacheData::GetFileSystemCache() const
{
    return fileSystemCache.get();
}
