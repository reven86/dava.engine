#include "Modules/DocumentsWatcherModule/DocumentsWatcherModule.h"
#include "Modules/DocumentsWatcherModule/DocumentsWatcherData.h"

#include "Modules/DocumentsModule/DocumentData.h"

#include <Engine/PlatformApi.h>
#include <Logger/Logger.h>

#include <QFileSystemWatcher>
#include <QApplication>

void DocumentsWatcherModule::PostInit()
{
    using namespace DAVA;
    using namespace TArc;
    DocumentsWatcherData* data = new DocumentsWatcherData();
    connections.AddConnection(data->watcher.get(), &QFileSystemWatcher::fileChanged, MakeFunction(this, &DocumentsWatcherModule::OnFileChanged));
    QApplication* app = PlatformApi::Qt::GetApplication();
    connections.AddConnection(app, &QApplication::applicationStateChanged, MakeFunction(this, &DocumentsWatcherModule::OnApplicationStateChanged));

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetGlobalContext();
    context->CreateData(std::unique_ptr<DocumentsWatcherData>(data));
}

void DocumentsWatcherModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetGlobalContext();
    context->DeleteData<DocumentsWatcherData>();
}

void DocumentsWatcherModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    DocumentData* data = context->GetData<DocumentData>();
    QString path = data->GetPackageAbsolutePath();

    DocumentsWatcherData* watcherData = context->GetData<DocumentsWatcherData>();
    watcherData->Watch(path);
}

void DocumentsWatcherModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    DocumentData* data = context->GetData<DocumentData>();
    QString path = data->GetPackageAbsolutePath();

    DocumentsWatcherData* watcherData = context->GetData<DocumentsWatcherData>();
    watcherData->Unwatch(path);
}

void DocumentsWatcherModule::OnFileChanged(const QString& path)
{
    using namespace DAVA::TArc;
    DataContext::ContextID id = GetContextByPath(path);
    changedDocuments.insert(id);
    DocumentData* data = GetAccessor()->GetContext(id)->GetData<DocumentData>();
    QFileInfo fileInfo(path);
    data->documentExists = fileInfo.exists();
    if (!data->CanSave() || qApp->applicationState() == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentsWatcherModule::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentsWatcherModule::ApplyFileChanges()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    QEGlobal::IDList changed;
    QEGlobal::IDList removed;
    for (DataContext::ContextID id : changedDocuments)
    {
        DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        DocumentData* data = context->GetData<DocumentData>();

        if (data->documentExists)
        {
            changed.insert(id);
        }
        else
        {
            removed.insert(id);
        }
    }
    changedDocuments.clear();
    //stack all confirm messages for remove and change
    QMutexLocker lock(&syncMutex);
    if (!changed.empty())
    {
        InvokeOperation(QEGlobal::ReloadDocuments.ID, changed);
    }
    if (!removed.empty())
    {
        InvokeOperation(QEGlobal::CloseDocuments.ID, removed);
    }
}

DAVA::TArc::DataContext::ContextID DocumentsWatcherModule::GetContextByPath(const QString& path) const
{
    using namespace DAVA::TArc;
    DataContext::ContextID ret = DataContext::Empty;
    GetAccessor()->ForEachContext([path, &ret](const DataContext& context) {
        DVASSERT(ret == DataContext::Empty);
        DocumentData* data = context.GetData<DocumentData>();
        if (data->GetPackageAbsolutePath() == path)
        {
            ret = context.GetID();
        }
    });
    DVASSERT(ret != DataContext::Empty);
    return ret;
}
