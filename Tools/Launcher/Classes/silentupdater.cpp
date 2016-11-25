#include "silentupdater.h"
#include "applicationmanager.h"

#include <QNetworkReply>
#include <QNetworkRequest>

SilentUpdater::SilentUpdater(ApplicationManager* appManager, SilentUpdateTask&& task_, QObject* parent)
    : QObject(parent)
    , applicationManager(appManager)
    , task(std::move(task_))
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &SilentUpdater::OnDownloadFinished);
    connect(networkManager, &QNetworkAccessManager::networkAccessibleChanged, this, &UpdateDialog::OnNetworkAccessibleChanged);

    QString url = task.newVersion.url;
    currentReply = networkManager->get(QNetworkRequest(QUrl(url)));
}

void SilentUpdater::OnDownloadFinished(QNetworkReply* reply)
{
    deleteLater();
    currentReply->deleteLater();
    CallBack onFinished = task.onFinished;
    if (currentReply->error() != QNetworkReply::NoError)
    {
        onFinished(false, currentReply->errorString());
        return;
    }
    QByteArray readedData = currentDownload->readAll();
    bool success = false;
    QString filePath = appManager->GetFileManager()->CreateZipFile(readedData, &success);
    if (success == false)
    {
        onFinished(false, tr("Can not create archive %1!").arg(filePath));
        return;
    }
    bool needRelaunch = false;

    QString appDir = appManager->GetApplicationDirectory(task.branchID, task.appID, false);
    if (task.currentVersion != nullptr)
    {
        QString localAppPath = ApplicationManager::GetLocalAppPath(task.currentVersion, task.appID);
        QString runPath = appDir + localAppPath;

        if (ProcessHelper::IsProcessRuning(runPath))
        {
            //i will burn in hell if i wlil not modify this code
            //AssetCacheServer is working on background, so we need to stop it and relaunch later
            if (task.appID == "AssetCacheServer")
            {
                AddLogValue(tr("Stopping AssetCacheServer..."));
                ProcessCommunication* processCommunication = appManager->GetProcessCommunicationModule();
                ProcessCommunication::eReply reply = processCommunication->SendSync(ProcessCommunication::eMessage::QUIT, runPath);
                if (reply == ProcessCommunication::eReply::ACCEPT)
                {
                    UpdateLastLogValue(tr("Waiting for AssetCacheServer will be stopped..."));
                    QElapsedTimer timer;
                    timer.start();
                    const int maxWaitTime = 10000; //10 secs;
                    bool isStillRunning = true;
                    while (timer.elapsed() < maxWaitTime && isStillRunning)
                    {
                        isStillRunning = ProcessHelper::IsProcessRuning(runPath);
                        QThread::msleep(100);
                    }
                    if (isStillRunning == false)
                    {
                        UpdateLastLogValue(tr("Asset cache server stopped"));
                        CompleteLog();
                        needRelaunch = true;
                    }
                    else
                    {
                        UpdateLastLogValue(tr("Asset cache server was not stopped till %1 seconds").arg(maxWaitTime / 1000));
                        BreakLog();
                        return;
                    }
                }
                else
                {
                    UpdateLastLogValue(tr("Can not stop asset cache server, last error was %1").arg(ProcessCommunication::GetReplyString(reply)));
                    BreakLog();
                    return;
                }
            }
            else
            {
                do
                {
                    if (ErrorMessenger::ShowRetryDlg(task.appID, runPath, true) == QMessageBox::Cancel)
                    {
                        AddLogValue(tr("Updating failed!"));
                        BreakLog();
                        return;
                    }
                } while (ProcessHelper::IsProcessRuning(runPath));
            }
        }
    }
    FileManager::DeleteDirectory(appDir);

    AddLogValue(tr("Unpacking archive..."));

    ui->cancelButton->setEnabled(false);
    ZipUtils::CompressedFilesAndSizes files;
    if (ListArchive(filePath, files)
        && UnpackArchive(filePath, appDir, files))
    {
        emit AppInstalled(task.branchID, task.appID, task.newVersion);
        UpdateLastLogValue("Unpack Complete!");
        CompleteLog();
    }
    else
    {
        UpdateLastLogValue("Unpack Fail!");
        BreakLog();
    }
    ui->cancelButton->setEnabled(true);
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());

    tasks.dequeue();
    if (needRelaunch)
    {
        appManager->RunApplication(task.branchID, task.appID, task.newVersion.id);
    }
    StartNextTask();
}

void SilentUpdater::OnNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    if (accessible == QNetworkAccessManager::NotAccessible)
    {
        currentReply->abort();
        currentReply->deleteLater();
        task.onFinished(false, "Network was disabled");
    }
}
