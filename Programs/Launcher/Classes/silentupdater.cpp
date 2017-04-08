#include "silentupdater.h"
#include "applicationmanager.h"
#include "filemanager.h"

#include <QNetworkReply>
#include <QNetworkRequest>

namespace SilentUpdaterDetails
{
class UpdateDialogZipFunctor : public ZipUtils::ZipOperationFunctor
{
public:
    UpdateDialogZipFunctor(const QString& errorMessage_, SilentUpdateTask::CallBack callBackFunction_)
        : errorMessage(errorMessage_)
        , callBackFunction(callBackFunction_)
    {
    }

    ~UpdateDialogZipFunctor() override = default;

private:
    void OnError(const ZipError& zipError) override
    {
        Q_ASSERT(zipError.error != ZipError::NO_ERRORS);
        callBackFunction(false, errorMessage + "\nerror text is: " + zipError.GetErrorString());
    }

private:
    QString errorMessage;
    SilentUpdateTask::CallBack callBackFunction;
};
}

struct SilentUpdater::TaskStarter
{
    TaskStarter(SilentUpdater* silentUpdater_)
        : silentUpdater(silentUpdater_)
    {
    }
    ~TaskStarter()
    {
        silentUpdater->StartNextTask();
    }

    SilentUpdater* silentUpdater = nullptr;
};

SilentUpdater::SilentUpdater(ApplicationManager* appManager, QObject* parent)
    : QObject(parent)
    , appManager(appManager)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::networkAccessibleChanged, this, &SilentUpdater::OnNetworkAccessibleChanged);
    connect(networkManager, &QNetworkAccessManager::finished, this, &SilentUpdater::OnDownloadFinished);
}

SilentUpdater::~SilentUpdater()
{
    canStartNextTask = false;
    tasks.clear();
    networkManager->disconnect(this);
    appManager = nullptr;
}

void SilentUpdater::AddTask(SilentUpdateTask&& task)
{
    tasks.append(std::move(task));
    StartNextTask();
}

void SilentUpdater::OnDownloadFinished(QNetworkReply* reply)
{
    reply->deleteLater();
    canStartNextTask = true;

    //this situation occurs on network error
    if (tasks.isEmpty())
    {
        return;
    }

    TaskStarter taskStarter(this);

    SilentUpdateTask task = tasks.dequeue();
    if (reply->error() != QNetworkReply::NoError)
    {
        task.onFinished(false, "Can not download remote archive, error is " + reply->errorString());
        return;
    }

    QByteArray readedData = reply->readAll();
    FileManager* fileManager = appManager->GetFileManager();
    QString downloadedFilePath = fileManager->GetTempDownloadFilePath(task.newVersion.url);
    bool archiveCreated = fileManager->CreateFileFromRawData(readedData, downloadedFilePath);
    if (archiveCreated == false)
    {
        task.onFinished(false, "Can not write archive to file " + downloadedFilePath);
        return;
    }
    QStringList applicationsToRestart;
    bool canRemoveCorrectly = appManager->PrepareToInstallNewApplication(task.branchID, task.appID, task.newVersion.isToolSet, true, applicationsToRestart);
    if (canRemoveCorrectly == false)
    {
        task.onFinished(false, "Can not remove installed applications");
        return;
    }

    QString appDir = appManager->GetApplicationDirectory(task.branchID, task.appID, task.newVersion.isToolSet, false);

    if (task.newVersion.url.endsWith("zip"))
    {
        ZipUtils::CompressedFilesAndSizes files;
        SilentUpdaterDetails::UpdateDialogZipFunctor listZipFunctor("Error while listing archive", task.onFinished);
        SilentUpdaterDetails::UpdateDialogZipFunctor unpackZipFunctor("Error while unpacking archive", task.onFinished);

        //if zip operation fails, callback will be triggered by zipFunctor
        if (ZipUtils::GetFileList(downloadedFilePath, files, listZipFunctor)
            && ZipUtils::UnpackZipArchive(downloadedFilePath, appDir, files, unpackZipFunctor))
        {
            appManager->OnAppInstalled(task.branchID, task.appID, task.newVersion);
            task.onFinished(true, "Success");
        }
    }
    else
    {
        QString fileName = fileManager->GetFileNameFromURL(task.newVersion.url);
        QString newFilePath = appDir + fileName;
        if (fileManager->MoveFileWithMakePath(downloadedFilePath, newFilePath))
        {
            appManager->OnAppInstalled(task.branchID, task.appID, task.newVersion);
            task.onFinished(true, "Success");
        }
        else
        {
            task.onFinished(false, QString("Can not move file from %1 to %2").arg(downloadedFilePath, newFilePath));
        }
    }

    FileManager::DeleteDirectory(fileManager->GetTempDirectory());
    for (const QString& appToRestart : applicationsToRestart)
    {
        appManager->RunApplication(task.branchID, appToRestart);
    }
}

void SilentUpdater::OnNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    bool isAccessible = accessible == QNetworkAccessManager::Accessible;
    canStartNextTask = isAccessible;
    if (isAccessible == false)
    {
        if (currentReply != nullptr)
        {
            tasks.dequeue();
            currentReply->abort();
            currentReply->deleteLater();
            currentReply = nullptr;

            SilentUpdateTask task = tasks.dequeue();
            task.onFinished(false, "Network was disabled");
        }
    }
}

void SilentUpdater::StartNextTask()
{
    if (canStartNextTask == false)
    {
        return;
    }
    if (tasks.isEmpty())
    {
        return;
    }
    const SilentUpdateTask& task = tasks.first();
    QString url = task.newVersion.url;
    canStartNextTask = false;
    currentReply = networkManager->get(QNetworkRequest(QUrl(url)));
}
