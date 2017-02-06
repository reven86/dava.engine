#pragma once

#include <TArc/DataProcessing/DataNode.h>

class QFileSystemWatcher;
class QString;

struct DocumentsWatcherData : public DAVA::TArc::DataNode
{
    DocumentsWatcherData();
    ~DocumentsWatcherData();

    void Watch(const QString& path);
    void Unwatch(const QString& path);

    std::unique_ptr<QFileSystemWatcher> watcher;
    DAVA_VIRTUAL_REFLECTION(DocumentsWatcherData, DAVA::TArc::DataNode);
};
