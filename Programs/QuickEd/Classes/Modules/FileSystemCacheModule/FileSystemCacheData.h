#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <QStringList>

class FileSystemCache;

class FileSystemCacheData : public DAVA::TArc::DataNode
{
public:
    FileSystemCacheData(const QStringList& extensions);
    ~FileSystemCacheData() override;

    QStringList GetFiles(const QString& extension) const;

private:
    friend class FileSystemCacheModule;

    FileSystemCache* GetFileSystemCache();
    std::unique_ptr<FileSystemCache> fileSystemCache;

    DAVA_VIRTUAL_REFLECTION(FileSystemCacheData, DAVA::TArc::DataNode);
};
