#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "filemanager.h"
#include "applicationmanager.h"
#include "errormessenger.h"
#include "processhelper.h"
#include "mainwindow.h"
#include "defines.h"

#include "QtHelpers/ProcessCommunication.h"

#include <QNetworkReply>
#include <QDir>
#include <QPushButton>
#include <QListWidget>
#include <QTreeView>
#include <QMessageBox>

namespace UpdateDialog_local
{
class UpdateDialogZipFunctor : public ZipUtils::ZipOperationFunctor
{
public:
    UpdateDialogZipFunctor(const QString& progressMessage_, const QString& finishMessage_, const QString& errorMessage_, UpdateDialog* dialog_, QProgressBar* progressBar_)
        : progressMessage(progressMessage_)
        , finishMessage(finishMessage_)
        , errorMessage(errorMessage_)
        , dialog(dialog_)
        , progressBar(progressBar_)
    {
    }

    ~UpdateDialogZipFunctor() override = default;

private:
    void OnStart() override
    {
        dialog->UpdateLastLogValue(progressMessage);
    }

    void OnProgress(int progress) override
    {
        progressBar->setValue(progress);
        dialog->UpdateLastLogValue(QString("%1...  %2%").arg(progressMessage).arg(progress));
    }

    void OnSuccess() override
    {
        dialog->UpdateLastLogValue(finishMessage);
    }

    void OnError(const ZipError& zipError) override
    {
        Q_ASSERT(zipError.error != ZipError::NO_ERRORS);
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_UNPACK, zipError.error, zipError.GetErrorString());
        dialog->UpdateLastLogValue(errorMessage);
        dialog->BreakLog();
    }

private:
    QString progressMessage;
    QString finishMessage;
    QString errorMessage;
    UpdateDialog* dialog = nullptr;
    QProgressBar* progressBar = nullptr;
};
}

UpdateDialog::UpdateDialog(const QQueue<UpdateTask>& taskQueue, ApplicationManager* _appManager, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::UpdateDialog)
    , networkManager(new QNetworkAccessManager(this))
    , tasks(taskQueue)
    , appManager(_appManager)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    //https://bugreports.qt.io/browse/QTBUG-51120
    ui->progressBar_downloading->setTextVisible(true);
    ui->progressBar_unpacking->setTextVisible(true);
#endif //Q_OS_MAC
    connect(networkManager, &QNetworkAccessManager::networkAccessibleChanged, this, &UpdateDialog::OnNetworkAccessibleChanged);
    connect(ui->cancelButton, &QPushButton::clicked, this, &UpdateDialog::OnCancelClicked);

    tasksCount = tasks.size();

    StartNextTask();
}

UpdateDialog::~UpdateDialog() = default;

void UpdateDialog::OnCancelClicked()
{
    if (currentDownload != nullptr)
        currentDownload->abort();
    FileManager* fileManager = appManager->GetFileManager();
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());

    close();
}

void UpdateDialog::UpdateButton()
{
    ui->cancelButton->setDefault(true);
    ui->cancelButton->setIcon(QIcon(":/Icons/ok.png"));
    ui->cancelButton->setFocusPolicy(Qt::StrongFocus);
}

void UpdateDialog::StartNextTask()
{
    if (!tasks.isEmpty())
    {
        ui->progressBar_downloading->setValue(0);
        ui->progressBar_unpacking->setValue(0);

        setWindowTitle(QString("Updating in progress... (%1/%2)").arg(tasksCount - tasks.size() + 1).arg(tasksCount));

        UpdateTask task = tasks.front();

        if (task.isRemoveBranch)
        {
            AddTopLogValue(QString("Removing branch %1:").arg(appManager->GetString(task.branchID)));

            if (appManager->RemoveBranch(task.branchID))
            {
                AddLogValue("Removing Complete!");
                CompleteLog();

                ui->progressBar_downloading->setValue(100);
            }
            else
            {
                AddLogValue("Removing Failed!");
                BreakLog();
            }

            tasks.dequeue();
            StartNextTask();
        }
        else
        {
            currentDownload = networkManager->get(QNetworkRequest(QUrl(task.newVersion.url)));
            connect(currentDownload, SIGNAL(finished()), this, SLOT(DownloadFinished()));
            connect(currentDownload, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(DownloadProgress(qint64, qint64)));

            AddTopLogValue(QString("%1 - %2:")
                           .arg(appManager->GetString(task.appID))
                           .arg(appManager->GetString(task.branchID))
                           );

            AddLogValue("Downloading file...");
        }
    }
    else
    {
        UpdateButton();
    }
}

void UpdateDialog::DownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal)
    {
        int percentage = (static_cast<double>(bytesReceived) / bytesTotal) * 100;
        ui->progressBar_downloading->setValue(percentage);

        UpdateLastLogValue(QString("Downloading file...  %1%").arg(percentage));
    }
}

void UpdateDialog::DownloadFinished()
{
    if (currentDownload == nullptr)
    {
        return;
    }
    QByteArray readedData = currentDownload->readAll();
    QNetworkReply::NetworkError error = currentDownload->error();
    QString errorString = currentDownload->errorString();

    currentDownload->deleteLater();
    currentDownload = nullptr;

    if (error == QNetworkReply::OperationCanceledError)
    {
        return;
    }
    else if (error != QNetworkReply::NoError)
    {
        UpdateLastLogValue("Download Fail!");
        BreakLog();

        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, error, errorString);
        return;
    }
    FileManager* fileManager = appManager->GetFileManager();
    const QString& archiveFilepath = fileManager->GetTempDownloadFilePath();
    QFile outputFile;
    outputFile.setFileName(archiveFilepath);
    outputFile.open(QFile::WriteOnly);
    outputFile.write(readedData);

    QString filePath = outputFile.fileName();
    outputFile.close();
    UpdateLastLogValue(tr("Download Complete!"));

    const UpdateTask& task = tasks.head();

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

void UpdateDialog::OnNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    if (currentDownload == nullptr)
    {
        return;
    }

    if (accessible == QNetworkAccessManager::NotAccessible)
    {
        currentDownload->deleteLater();
        currentDownload = nullptr;

        UpdateLastLogValue("Download Fail!");
        BreakLog();

        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, -1, tr("Network is unaccessible"));
    }
}

bool UpdateDialog::ListArchive(const QString& archivePath, ZipUtils::CompressedFilesAndSizes& files)
{
    UpdateDialog_local::UpdateDialogZipFunctor functor("Retreiveing archive info...", "Unpacking archive...", tr("Archive not found or damaged!"), this, nullptr);
    return ZipUtils::GetFileList(archivePath, files, functor);
}

bool UpdateDialog::UnpackArchive(const QString& archivePath, const QString& outDir, const ZipUtils::CompressedFilesAndSizes& files)
{
    UpdateDialog_local::UpdateDialogZipFunctor functor(tr("Unpacking archive..."), tr("Archive unpacked!"), tr("Unpacking failed!"), this, ui->progressBar_unpacking);
    return ZipUtils::UnpackZipArchive(archivePath, outDir, files, functor);
}

void UpdateDialog::AddTopLogValue(const QString& log)
{
    if (currentTopLogItem != nullptr)
    {
        currentTopLogItem->setTextColor(0, LOG_COLOR_COMPLETE);
    }
    currentTopLogItem = new QTreeWidgetItem();
    currentTopLogItem->setText(0, log);
    currentTopLogItem->setTextColor(0, LOG_COLOR_PROGRESS);
    ui->treeWidget->addTopLevelItem(currentTopLogItem);

    ui->treeWidget->expandAll();
    ui->treeWidget->scrollToBottom();
}

void UpdateDialog::AddLogValue(const QString& log)
{
    if (currentLogItem != nullptr)
    {
        currentLogItem->setTextColor(0, LOG_COLOR_COMPLETE);
    }
    currentLogItem = new QTreeWidgetItem();
    currentLogItem->setText(0, log);
    currentLogItem->setTextColor(0, LOG_COLOR_PROGRESS);

    currentTopLogItem->addChild(currentLogItem);

    ui->treeWidget->expandAll();
    ui->treeWidget->scrollToBottom();
}

void UpdateDialog::UpdateLastLogValue(const QString& log)
{
    if (currentLogItem != nullptr)
    {
        currentLogItem->setText(0, log);
    }
}

void UpdateDialog::BreakLog()
{
    if (currentTopLogItem != nullptr)
    {
        currentTopLogItem->setTextColor(0, LOG_COLOR_FAIL);
        currentTopLogItem = nullptr;
    }
    if (currentLogItem != nullptr)
    {
        currentLogItem->setTextColor(0, LOG_COLOR_FAIL);
        currentLogItem = nullptr;
    }
}

void UpdateDialog::CompleteLog()
{
    if (currentTopLogItem != nullptr)
    {
        currentTopLogItem->setTextColor(0, LOG_COLOR_COMPLETE);
        currentTopLogItem = nullptr;
    }
    if (currentLogItem != nullptr)
    {
        currentLogItem->setTextColor(0, LOG_COLOR_COMPLETE);
        currentLogItem = nullptr;
    }
}
