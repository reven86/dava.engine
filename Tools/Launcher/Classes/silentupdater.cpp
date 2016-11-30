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


    SilentUpdateTask task = tasks.dequeue();
    if (reply->error() != QNetworkReply::NoError)
    {
        task.onFinished(false, "Can not download remote archive, error is " + reply->errorString());
        return;
    }

    QByteArray readedData = reply->readAll();
    bool success = false;
    FileManager* fileManager = appManager->GetFileManager();
    QString archivePath = fileManager->CreateZipFile(readedData, &success);
    if (success == false)
    {
        task.onFinished(false, "Can not write archive to file " + archivePath);
        return;
    }
    QStringList applicationsToRestart;
    bool canRemoveCorrectrly = appManager->PrepareToInstallNewApplication(task.branchID, task.appID, task.newVersion.isToolSet, true, applicationsToRestart);
    if (canRemoveCorrectrly == false)
    {
        task.onFinished(false, "Can not remove installed applications");
        return;
    }

    QString appDir = appManager->GetApplicationDirectory(task.branchID, task.appID, task.newVersion.isToolSet, false);

    ZipUtils::CompressedFilesAndSizes files;
    SilentUpdaterDetails::UpdateDialogZipFunctor listZipFunctor("Error while listing archive", task.onFinished);
    SilentUpdaterDetails::UpdateDialogZipFunctor unpackZipFunctor("Error while unpacking archive", task.onFinished);

    //if zip operation fails, callback will be triggered by zipFunctor
    if (ZipUtils::GetFileList(archivePath, files, listZipFunctor)
        && ZipUtils::UnpackZipArchive(archivePath, appDir, files, unpackZipFunctor))
    {
        appManager->OnAppInstalled(task.branchID, task.appID, task.newVersion);
        task.onFinished(true, "Success");
    }

    FileManager::DeleteDirectory(fileManager->GetTempDirectory());
    for (const QString& appToRestart : applicationsToRestart)
    {
        appManager->RunApplication(task.branchID, appToRestart);
    }
    StartNextTask();
}

void SilentUpdater::OnNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    if (accessible == QNetworkAccessManager::NotAccessible)
    {
        if (currentReply != nullptr)
        {
            currentReply->abort();
            currentReply->deleteLater();
            currentReply = nullptr;
            while (tasks.isEmpty() == false)
            {
                SilentUpdateTask task = tasks.dequeue();
                task.onFinished(false, "Network was disabled");
            }
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
    const SilentUpdateTask& task = tasks.last();
    QString url = task.newVersion.url;
    canStartNextTask = false;
    currentReply = networkManager->get(QNetworkRequest(QUrl(url)));
}
