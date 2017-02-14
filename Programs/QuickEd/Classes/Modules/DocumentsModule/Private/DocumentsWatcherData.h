#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <TArc/DataProcessing/DataContext.h>

class QFileSystemWatcher;
class QString;

class DocumentsWatcherData : public DAVA::TArc::DataNode
{
public:
    DocumentsWatcherData();
    ~DocumentsWatcherData();

private:
    friend class DocumentsModule;

    void Watch(const QString& path);
    void Unwatch(const QString& path);

    //store changed ids to iterate them when application will be active
    DAVA::Set<DAVA::TArc::DataContext::ContextID> changedDocuments;
    std::unique_ptr<QFileSystemWatcher> watcher;
    DAVA_VIRTUAL_REFLECTION(DocumentsWatcherData, DAVA::TArc::DataNode);
};
