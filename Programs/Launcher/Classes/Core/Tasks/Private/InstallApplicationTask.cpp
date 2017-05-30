#include "Core/Tasks/InstallApplicationTask.h"
#include "Core/Tasks/DownloadTask.h"
#include "Core/Tasks/RemoveApplicationTask.h"
#include "Core/Tasks/UnzipTask.h"

#include "Core/ApplicationManager.h"

#include "Utils/AppsCommandsSender.h"
#include "Utils/FileManager.h"

#include <QNetworkReply>
#include <QEventLoop>

InstallApplicationTask::InstallApplicationTask(ApplicationManager* appManager, const InstallApplicationParams& params_)
    : AsyncChainTask(appManager)
    , params(params_)
{
}

QString InstallApplicationTask::GetDescription() const
{
    return QObject::tr("Installing application %1").arg(appManager->GetAppName(params.app, params.newVersion.isToolSet));
}

void InstallApplicationTask::Run()
{
    QString description = QObject::tr("Downloading application %1").arg(appManager->GetAppName(params.app, params.newVersion.isToolSet));
    std::unique_ptr<BaseTask> task = appManager->CreateTask<DownloadTask>(description, params.newVersion.url);
    appManager->AddTaskWithNotifier(std::move(task), notifier);
}

int InstallApplicationTask::GetSubtasksCount() const
{
    return NeedUnpack() ? 2 : 1;
}

void InstallApplicationTask::OnFinished(const BaseTask* task)
{
    if (task->HasError())
    {
        //if unpacking is cancelled - mainWindow will not refresh apps
        appManager->GetMainWindow()->RefreshApps();
        emit Finished();
    }
    else
    {
        switch (state)
        {
        case LOADING:
            notifier.IncrementStep();
            OnLoaded(task);
            break;
        case UNPACKING:
            Install();
            break;
        case POST_INSTALL:
            OnInstalled();
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
}

void InstallApplicationTask::OnLoaded(const BaseTask* task)
{
    FileManager* fileManager = appManager->GetFileManager();
    QString filePath = fileManager->GetTempDownloadFilePath(params.newVersion.url);

    Q_ASSERT(task->GetTaskType() == BaseTask::DOWNLOAD_TASK);
    const DownloadTask* downloadTask = static_cast<const DownloadTask*>(task);
    Q_ASSERT(downloadTask->GetLoadedData().empty() == false);

    //move this code to DownloadTask later
    bool archiveCreated = fileManager->CreateFileFromRawData(downloadTask->GetLoadedData().front(), filePath);
    if (archiveCreated == false)
    {
        SetError(QObject::tr("Can not create file %1!").arg(filePath));
        emit Finished();
        return;
    }
    state = UNPACKING;
    applicationsToRestart = GetApplicationsToRestart(params.branch, params.app);

    //remove application if was installed
    Application* localApp = appManager->GetLocalConfig()->GetApplication(params.branch, params.app);
    if (localApp != nullptr)
    {
        AppVersion* localVersion = localApp->GetVersion(0);
        if (localVersion != nullptr)
        {
            std::unique_ptr<BaseTask> task = appManager->CreateTask<RemoveApplicationTask>(params.branch, params.app);
            appManager->AddTaskWithNotifier(std::move(task), notifier);
            return;
        }
    }
    //will not start remove task, can start install manually
    Install();
}

void InstallApplicationTask::Install()
{
    FileManager* fileManager = appManager->GetFileManager();
    QString filePath = fileManager->GetTempDownloadFilePath(params.newVersion.url);

    QString appDirPath = appManager->GetApplicationDirectory(params.branch, params.app, params.newVersion.isToolSet, false);
    state = POST_INSTALL;
    if (NeedUnpack())
    {
        std::unique_ptr<BaseTask> task = appManager->CreateTask<UnzipTask>(filePath, appDirPath);
        appManager->AddTaskWithNotifier(std::move(task), notifier);
        return;
    }
    else
    {
        QString newFilePath = appDirPath + fileManager->GetFileNameFromURL(params.newVersion.url);
        if (fileManager->MoveFileWithMakePath(filePath, newFilePath))
        {
            OnInstalled();
        }
        else
        {
            SetError(QObject::tr("Moving downloaded application failed"));
            emit Finished();
        }
    }
}

void InstallApplicationTask::OnInstalled()
{
    appManager->OnAppInstalled(params.branch, params.app, params.newVersion);

    FileManager* fileManager = appManager->GetFileManager();
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());
    for (const QString& appToRestart : applicationsToRestart)
    {
        appManager->RunApplication(params.branch, appToRestart);
    }
    emit Finished();
}

QStringList InstallApplicationTask::GetApplicationsToRestart(const QString& branchID, const QString& appID)
{
    QStringList appList;
    const AppVersion* installedVersion = appManager->GetInstalledVersion(branchID, appID);

    if (installedVersion == nullptr)
    {
        return appList;
    }

    QStringList appNames;
    if (installedVersion->isToolSet)
    {
        QStringList toolsetApps = appManager->GetLocalConfig()->GetTranslatedToolsetApplications();
        for (auto iter = toolsetApps.begin(); iter != toolsetApps.end(); ++iter)
        {
            appNames << *iter;
        }
    }
    else
    {
        appNames << appID;
    }

    for (const QString& appName : appNames)
    {
        if (appManager->CanTryStopApplication(appName))
        {
            QString dirPath = appManager->GetApplicationDirectory(branchID, appID, installedVersion->isToolSet, false);
            QString localPath = appManager->GetLocalAppPath(installedVersion, appID);
            if (appManager->GetAppsCommandsSender()->HostIsAvailable(dirPath + localPath))
            {
                appList << appName;
            }
        }
    }
    return appList;
}

bool InstallApplicationTask::NeedUnpack() const
{
    return params.newVersion.url.endsWith("zip");
}
