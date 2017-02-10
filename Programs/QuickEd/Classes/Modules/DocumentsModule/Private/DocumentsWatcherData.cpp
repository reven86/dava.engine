#include "Modules/DocumentsModule/DocumentsWatcherData.h"

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
        QString errorMessage = "Can not watch document " + path;
        DAVA::Logger::Error(errorMessage.toStdString().c_str());
    }
}

void DocumentsWatcherData::Unwatch(const QString& path)
{
    if (watcher->removePath(path) == false)
    {
        QString errorMessage = "Can not unwatch document " + path;
        DAVA::Logger::Error(errorMessage.toStdString().c_str());
    }
}
