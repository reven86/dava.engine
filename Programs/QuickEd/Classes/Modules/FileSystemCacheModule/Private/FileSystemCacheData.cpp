#include "Modules/FileSystemCacheModule/FileSystemCacheData.h"
#include <QtTools/ProjectInformation/FileSystemCache.h>

DAVA_VIRTUAL_REFLECTION_IMPL(FileSystemCacheData)
{
    DAVA::ReflectionRegistrator<FileSystemCacheData>::Begin()
    .ConstructorByPointer<QStringList>()
    .End();
}

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
