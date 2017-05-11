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
    appManager->AddTaskWithCB<DownloadTask>(WrapCB(std::bind(&SelfUpdateTask::OnLoaded, this, std::placeholders::_1)), description, url);
}

void SelfUpdateTask::OnLoaded(const BaseTask* task)
{
    Q_ASSERT(task->GetTaskType() == BaseTask::DOWNLOAD_TASK);
    const DownloadTask* downloadTask = static_cast<const DownloadTask*>(task);
    Q_ASSERT(downloadTask->GetLoadedData().isEmpty() == false);
    FileManager* fileManager = appManager->GetFileManager();
    QString filePath = fileManager->GetTempDownloadFilePath(url);
    bool archiveCreated = fileManager->CreateFileFromRawData(downloadTask->GetLoadedData().first(), filePath);
    if (archiveCreated == false)
    {
        SetError(QObject::tr("Can not create archive %1!").arg(filePath));
        return;
    }
    QString tempArchiveFilePath = fileManager->GetTempDownloadFilePath(url);
    QString selfUpdateDirPath = fileManager->GetSelfUpdateTempDirectory();

    appManager->AddTaskWithCB<UnzipTask>(WrapCB(std::bind(&SelfUpdateTask::OnUnpacked, this)), tempArchiveFilePath, selfUpdateDirPath);
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
