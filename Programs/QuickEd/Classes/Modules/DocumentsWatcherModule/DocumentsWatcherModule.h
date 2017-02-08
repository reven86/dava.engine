#pragma once

#include "Application/QEGlobal.h"
#include <TArc/Core/ClientModule.h>

#include <TArc/Utils/QtConnections.h>

#include <QMutex>

class DocumentsWatcherModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key);

    void OnContextCreated(DAVA::TArc::DataContext* context);
    void OnContextDeleted(DAVA::TArc::DataContext* context);

    void OnFileChanged(const QString& path);
    void OnApplicationStateChanged(Qt::ApplicationState state);

    void ApplyFileChanges();

    void OnFilesRemoved(const QEGlobal::IDList& removedFiles);

    DAVA::TArc::DataContext::ContextID GetContextByPath(const QString& path) const;

    QEGlobal::IDList changedDocuments;

    DAVA::TArc::QtConnections connections;

    //this mutex is used to control situation, when some files changed or removed while modal dialog "confirm changes" is displayed
    QMutex syncMutex;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DocumentsWatcherModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<DocumentsWatcherModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
