#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <QStringList>

class FileSystemCache;

class FileSystemCacheData : public DAVA::TArc::DataNode
{
public:
    FileSystemCacheData(const QStringList& extensions);
    ~FileSystemCacheData() override;

    //TODO: remove this public getter; Move FindWidget to TArc and create here public const method GetFiles(const String &extension);
    const FileSystemCache* GetFileSystemCache() const;

private:
    friend class FileSystemCacheModule;

    FileSystemCache* GetFileSystemCache();
    std::unique_ptr<FileSystemCache> fileSystemCache;

    DAVA_VIRTUAL_REFLECTION(FileSystemCacheData, DAVA::TArc::DataNode);
};
