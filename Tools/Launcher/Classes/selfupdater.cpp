#include "selfupdater.h"
#include "ui_selfupdater.h"
#include "filemanager.h"
#include "ziputils.h"
#include "processhelper.h"
#include "errormessanger.h"
#include <QProcess>

namespace SelfUpdater_local
{
class SelfUpdaterZipFunctor : public ZipUtils::ZipOperationFunctor
{
public:
    SelfUpdaterZipFunctor() = default;
    ~SelfUpdaterZipFunctor() override = default;

private:
    void OnError(const ZipError& zipError) override
    {
        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_UNPACK, zipError.error, zipError.GetErrorString());
    }
};
}

SelfUpdater::SelfUpdater(const QString& arcUrl, QNetworkAccessManager* accessManager, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::SelfUpdater)
    , archiveUrl(arcUrl)
    , networkManager(accessManager)
{
    ui->setupUi(this);

    connect(this, SIGNAL(StartUpdating()), this, SLOT(OnStartUpdating()));

    emit StartUpdating();
}

SelfUpdater::~SelfUpdater() = default;

void SelfUpdater::OnStartUpdating()
{
    currentDownload = networkManager->get(QNetworkRequest(QUrl(archiveUrl)));

    connect(currentDownload, SIGNAL(finished()), this, SLOT(DownloadFinished()));
    connect(currentDownload, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(NetworkError(QNetworkReply::NetworkError)));
}

void SelfUpdater::NetworkError(QNetworkReply::NetworkError code)
{
    lastErrorDesrc = currentDownload->errorString();
    lastErrorCode = code;

    currentDownload->deleteLater();
    currentDownload = nullptr;
}

void SelfUpdater::DownloadFinished()
{
    if (currentDownload)
    {
        FileManager::Instance()->ClearTempDirectory();

        const QString& archiveFilePath = FileManager::Instance()->GetTempDownloadFilepath();
        const QString& tempDirPath = FileManager::Instance()->GetTempDirectory();
        const QString& appDirPath = FileManager::Instance()->GetLauncherDirectory();
        const QString& selfUpdateDirPath = FileManager::Instance()->GetSelfUpdateTempDirectory();

        QFile archiveFile(archiveFilePath);
        archiveFile.open(QFile::WriteOnly);
        archiveFile.write(currentDownload->readAll());
        archiveFile.close();

        currentDownload->deleteLater();
        currentDownload = nullptr;

        ZipUtils::CompressedFilesAndSizes files;
        SelfUpdater_local::SelfUpdaterZipFunctor functor;
        if (ZipUtils::GetFileList(archiveFilePath, files, functor)
            && ZipUtils::TestZipArchive(archiveFilePath, files, functor)
            && ZipUtils::UnpackZipArchive(archiveFilePath, selfUpdateDirPath, files, functor)
            )
        {
            FileManager::Instance()->MoveFilesOnlyToDirectory(appDirPath, tempDirPath);
            FileManager::Instance()->MoveFilesOnlyToDirectory(selfUpdateDirPath, appDirPath);
            FileManager::Instance()->DeleteDirectory(selfUpdateDirPath);
            FileManager::Instance()->ClearTempDirectory();
            ErrorMessanger::Instance()->ShowNotificationDlg("Launcher was updated. Please, relaunch application.");
            qApp->exit();
        }
        else
        {
            FileManager::Instance()->DeleteDirectory(selfUpdateDirPath);
            setResult(QDialog::Rejected);
            close();
        }
    }
    else if (lastErrorCode != QNetworkReply::OperationCanceledError)
    {
        setResult(QDialog::Rejected);
        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_NETWORK, lastErrorCode, lastErrorDesrc);
        close();
    }
}
