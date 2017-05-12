#include "Core/Tasks/SelfUpdateTask.h"
#include "Core/Tasks/UnzipTask.h"
#include "Core/Tasks/DownloadTask.h"

#include "Core/ApplicationManager.h"
#include "Core/ApplicationQuitController.h"

#include "Utils/FileManager.h"

#include <QApplication>

SelfUpdateTask::SelfUpdateTask(ApplicationManager* appManager, ApplicationQuitController* quitController_, const QString& url_)
    : AsyncChainTask(appManager)
    , quitController(quitController_)
    , url(url_)
{
}

QString SelfUpdateTask::GetDescription() const
{
    return QObject::tr("Starting self update");
}

void SelfUpdateTask::Run()
{
    QString description = QObject::tr("Loading new launcher");
    std::unique_ptr<BaseTask> task = appManager->CreateTask<DownloadTask>(description, url);
    task->SetOnSuccessCallback(std::bind(&SelfUpdateTask::OnLoaded, this, std::placeholders::_1));
    appManager->AddTask(std::move(task));
}

void SelfUpdateTask::OnLoaded(const BaseTask* task)
{
    Q_ASSERT(task->GetTaskType() == BaseTask::DOWNLOAD_TASK);
    const DownloadTask* downloadTask = static_cast<const DownloadTask*>(task);
    Q_ASSERT(downloadTask->GetLoadedData().empty() == false);
    FileManager* fileManager = appManager->GetFileManager();
    QString filePath = fileManager->GetTempDownloadFilePath(url);
    bool archiveCreated = fileManager->CreateFileFromRawData(downloadTask->GetLoadedData().front(), filePath);
    if (archiveCreated == false)
    {
        SetError(QObject::tr("Can not create archive %1!").arg(filePath));
        return;
    }
    QString tempArchiveFilePath = fileManager->GetTempDownloadFilePath(url);
    QString selfUpdateDirPath = fileManager->GetSelfUpdateTempDirectory();

    std::unique_ptr<BaseTask> zipTask = appManager->CreateTask<UnzipTask>(tempArchiveFilePath, selfUpdateDirPath);
    zipTask->SetOnSuccessCallback(std::bind(&SelfUpdateTask::OnUnpacked, this));
    appManager->AddTask(std::move(zipTask));
}

void SelfUpdateTask::OnUnpacked()
{
    FileManager* fileManager = appManager->GetFileManager();
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());
    QString tempDir = fileManager->GetTempDirectory(); //create temp directory

    QString appDirPath = fileManager->GetLauncherDirectory();
    QString selfUpdateDirPath = fileManager->GetSelfUpdateTempDirectory();

    //remove old launcher files except download folder, temp folder and update folder
    if (fileManager->MoveLauncherRecursively(appDirPath, tempDir, this)
        && fileManager->MoveLauncherRecursively(selfUpdateDirPath, appDirPath, this))
    {
        quitController->requireRestart = true;
        qApp->quit();
    }

    Finished();
}
