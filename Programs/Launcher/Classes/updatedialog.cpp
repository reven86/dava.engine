#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "filemanager.h"
#include "applicationmanager.h"
#include "errormessenger.h"
#include "mainwindow.h"
#include "defines.h"

#include "QtHelpers/ProcessHelper.h"

#include <QNetworkReply>
#include <QDir>
#include <QPushButton>
#include <QListWidget>
#include <QTreeView>
#include <QMessageBox>

namespace UpdateDialogDetails
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
    accept();
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

            QString appName = task.newVersion.isToolSet ? "Toolset" : task.appID;
            AddTopLogValue(QString("%1 - %2:")
                           .arg(appManager->GetString(appName))
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
    QNetworkReply::NetworkError error = currentDownload->error();
    QString errorString = currentDownload->errorString();
    QByteArray readedData = currentDownload->readAll();

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
    const UpdateTask& task = tasks.head();

    FileManager* fileManager = appManager->GetFileManager();
    QString filePath = fileManager->GetTempDownloadFilePath(task.newVersion.url);
    bool archiveCreated = fileManager->CreateFileFromRawData(readedData, filePath);
    if (archiveCreated == false)
    {
        UpdateLastLogValue(tr("Can not create archive %1!").arg(filePath));
        BreakLog();
        return;
    }
    UpdateLastLogValue(tr("Download Complete!"));

    QStringList applicationsToRestart;
    bool canRemoveCorrectrly = appManager->PrepareToInstallNewApplication(task.branchID, task.appID, task.newVersion.isToolSet, false, applicationsToRestart);
    if (canRemoveCorrectrly == false)
    {
        UpdateLastLogValue("Removing applications Failed!");
        BreakLog();
        return;
    }

    //create path to a new version directory
    QString appDir = appManager->GetApplicationDirectory(task.branchID, task.appID, task.newVersion.isToolSet, false);
    ui->cancelButton->setEnabled(false);

    if (task.newVersion.url.endsWith("zip"))
    {
        AddLogValue(tr("Unpacking archive..."));
        ZipUtils::CompressedFilesAndSizes files;
        if (ListArchive(filePath, files)
            && UnpackArchive(filePath, appDir, files))
        {
            appManager->OnAppInstalled(task.branchID, task.appID, task.newVersion);
            UpdateLastLogValue("Unpack Complete!");
            CompleteLog();
        }
        else
        {
            UpdateLastLogValue("Unpack Fail!");
            BreakLog();
        }
    }
    else
    {
        AddLogValue(tr("Moving file archive..."));
        QString newFilePath = appDir + fileManager->GetFileNameFromURL(task.newVersion.url);
        if (fileManager->MoveFileWithMakePath(filePath, newFilePath))
        {
            appManager->OnAppInstalled(task.branchID, task.appID, task.newVersion);
            UpdateLastLogValue("Move Complete!");
            CompleteLog();
        }
        else
        {
            UpdateLastLogValue("Moving Fail!");
            BreakLog();
        }
    }
    ui->cancelButton->setEnabled(true);
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());

    for (const QString& appToRestart : applicationsToRestart)
    {
        appManager->RunApplication(task.branchID, appToRestart);
    }
    tasks.dequeue();

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
    UpdateDialogDetails::UpdateDialogZipFunctor functor("Retreiveing archive info...", "Unpacking archive...", tr("Archive not found or damaged!"), this, nullptr);
    return ZipUtils::GetFileList(archivePath, files, functor);
}

bool UpdateDialog::UnpackArchive(const QString& archivePath, const QString& outDir, const ZipUtils::CompressedFilesAndSizes& files)
{
    UpdateDialogDetails::UpdateDialogZipFunctor functor(tr("Unpacking archive..."), tr("Archive unpacked!"), tr("Unpacking failed!"), this, ui->progressBar_unpacking);
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
