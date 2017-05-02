#include "selfupdater.h"
#include "ui_selfupdater.h"
#include "filemanager.h"
#include "ziputils.h"
#include "errormessenger.h"

#include "QtHelpers/ProcessHelper.h"

#include <QProcess>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>
#include <QDir>

namespace SelfUpdater_local
{
class SelfUpdaterZipFunctor : public ZipUtils::ZipOperationFunctor
{
public:
    SelfUpdaterZipFunctor(QProgressBar* bar_)
        : bar(bar_)
    {
    }
    ~SelfUpdaterZipFunctor() override = default;

private:
    void OnError(const ZipError& zipError) override
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_UNPACK, zipError.error, zipError.GetErrorString());
    }
    void OnProgress(int value) override
    {
        bar->setValue(value);
    }
    QProgressBar* bar = nullptr;
};
}

SelfUpdater::SelfUpdater(FileManager* fileManager_, const QString& arcUrl, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::SelfUpdater)
    , archiveUrl(arcUrl)
    , networkManager(new QNetworkAccessManager(this))
    , fileManager(fileManager_)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    //https://bugreports.qt.io/browse/QTBUG-51120
    ui->progressBar_processing->setTextVisible(true);
    ui->progressBar_downoading->setTextVisible(true);
#endif //Q_OS_MAC
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    currentDownload = networkManager->get(QNetworkRequest(QUrl(archiveUrl)));
    connect(currentDownload, &QNetworkReply::downloadProgress, this, &SelfUpdater::DownloadProgress);
    connect(currentDownload, &QNetworkReply::finished, this, &SelfUpdater::DownloadFinished);
    connect(currentDownload, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &SelfUpdater::NetworkError);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, currentDownload, &QNetworkReply::abort);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

SelfUpdater::~SelfUpdater() = default;

void SelfUpdater::NetworkError(QNetworkReply::NetworkError code)
{
    lastErrorDesrc = currentDownload->errorString();
    lastErrorCode = code;

    currentDownload->deleteLater();
    currentDownload = nullptr;
}
struct DirCleaner
{
    DirCleaner(FileManager* fileManager_)
        : fileManager(fileManager_)
    {
        Q_ASSERT(fileManager != nullptr);
    }

    ~DirCleaner()
    {
        FileManager::DeleteDirectory(fileManager->GetTempDirectory());
        FileManager::DeleteDirectory(fileManager->GetSelfUpdateTempDirectory());
    }
    FileManager* fileManager;
};

SelfUpdater::UpdateError SelfUpdater::ProcessLauncherUpdate()
{
    QString appDirPath = fileManager->GetLauncherDirectory();
    QString tempArchiveFilePath = fileManager->GetTempDownloadFilePath(archiveUrl);
    QString selfUpdateDirPath = fileManager->GetSelfUpdateTempDirectory();

    ZipUtils::CompressedFilesAndSizes files;
    SelfUpdater_local::SelfUpdaterZipFunctor functor(ui->progressBar_processing);
    if ((ZipUtils::GetFileList(tempArchiveFilePath, files, functor)
         && ZipUtils::UnpackZipArchive(tempArchiveFilePath, selfUpdateDirPath, files, functor)) == false)
    {
        return ARCHIVE_ERROR;
    }

    FileManager::DeleteDirectory(fileManager->GetTempDirectory());
    QString tempDir = fileManager->GetTempDirectory(); //create temp directory
    //remove old launcher files except download folder, temp folder and update folder
    if (fileManager->MoveLauncherRecursively(appDirPath, tempDir)
        && fileManager->MoveLauncherRecursively(selfUpdateDirPath, appDirPath))
    {
        return NO_ERRORS;
    }
    else
    {
        return MOVE_FILES_ERROR;
    }
}

void SelfUpdater::DownloadFinished()
{
    DirCleaner raiiDirCleaner(fileManager);
    if (currentDownload)
    {
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

        UpdateError err = NO_ERRORS;
        QByteArray data = currentDownload->readAll();

        currentDownload->deleteLater();
        currentDownload = nullptr;
        //create an archive with a new version
        QString tempArchiveFilePath = fileManager->GetTempDownloadFilePath(archiveUrl);
        bool archiveCreated = fileManager->CreateFileFromRawData(data, tempArchiveFilePath);
        if (archiveCreated)
        {
            err = ProcessLauncherUpdate();
        }
        else
        {
            err = ARCHIVE_ERROR;
        }
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        if (err == NO_ERRORS)
        {
            ui->label->setText(tr("Launcher was updated!\nPlease relaunch application!"));
            connect(this, &QDialog::finished, qApp, &QApplication::quit);
        }
        else
        {
            ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_UPDATE, tr("Error occurred while updating launcher:\n%1").arg(ErrorString(err)));
            ui->label->setText(tr("Launcher was not updated!"));
            return;
        }
    }
    //network error
    else if (lastErrorCode != QNetworkReply::OperationCanceledError)
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, lastErrorCode, lastErrorDesrc);
        reject();
    }
    //cancelled
    else
    {
        accept();
    }
}

void SelfUpdater::DownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal)
    {
        double percentage = (static_cast<double>(bytesReceived) / bytesTotal) * 100.0f;
        ui->progressBar_downoading->setValue(static_cast<int>(percentage));
    }
}

QString SelfUpdater::ErrorString(UpdateError err) const
{
    switch (err)
    {
    case NO_ERRORS:
        return "";
    case ARCHIVE_ERROR:
        return tr("Error occurred while work with new version archive");
    case MOVE_FILES_ERROR:
        return tr("Can not move launcher files! It can be critical, call administrator please");
    case INFO_FILE_ERROR:
        return tr("Can not create file with launcher info");
    }
    return tr("Unrecognized error");
}
