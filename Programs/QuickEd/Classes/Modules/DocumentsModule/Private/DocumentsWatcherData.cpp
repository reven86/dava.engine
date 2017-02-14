#include "Modules/DocumentsModule/Private/DocumentsWatcherData.h"

#include <Logger/Logger.h>

#include <QFileSystemWatcher>

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentsWatcherData)
{
}

DocumentsWatcherData::DocumentsWatcherData()
{
    watcher.reset(new QFileSystemWatcher());
}

DocumentsWatcherData::~DocumentsWatcherData() = default;

void DocumentsWatcherData::Watch(const QString& path)
{
    if (watcher->addPath(path) == false)
    {
        DAVA::Logger::Error("Can not watch document %s", path.toStdString().c_str());
    }
}

void DocumentsWatcherData::Unwatch(const QString& path)
{
    if (watcher->removePath(path) == false)
    {
        DAVA::Logger::Error("Can not unwatch document %s", path.toStdString().c_str());
    }
}
