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

QStringList FileSystemCacheData::GetFiles(const QString& extension) const
{
    return fileSystemCache->GetFiles(extension);
}

FileSystemCache* FileSystemCacheData::GetFileSystemCache()
{
    return fileSystemCache.get();
}
