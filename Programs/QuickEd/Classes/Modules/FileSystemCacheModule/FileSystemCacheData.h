#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <QStringList>

class FileSystemCache;

class FileSystemCacheData : public DAVA::TArc::DataNode
{
public:
    FileSystemCacheData(const QStringList& extensions);
    ~FileSystemCacheData() override;

    FileSystemCache* GetFileSystemCache();
    const FileSystemCache* GetFileSystemCache() const;

private:
    std::unique_ptr<FileSystemCache> fileSystemCache;

    DAVA_VIRTUAL_REFLECTION(FileSystemCacheData, DAVA::TArc::DataNode);
};
