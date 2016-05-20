/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    =====================================================================================*/


#include "selfupdater.h"
#include "ui_selfupdater.h"
#include "filemanager.h"
#include "ziputils.h"
#include "processhelper.h"
#include "errormessenger.h"
#include <QProcess>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>

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

SelfUpdater::SelfUpdater(const QString& arcUrl, QNetworkAccessManager* accessManager, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::SelfUpdater)
    , archiveUrl(arcUrl)
    , networkManager(accessManager)
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
    ~DirCleaner()
    {
        FileManager::DeleteDirectory(FileManager::GetTempDirectory());
        FileManager::DeleteDirectory(FileManager::GetSelfUpdateTempDirectory());
    }
};

void SelfUpdater::DownloadFinished()
{
    DirCleaner raiiDirCleaner;
    if (currentDownload)
    {
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

        QString appDirPath = FileManager::GetLauncherDirectory();
        QString tempArchiveFilePath = FileManager::GetTempDownloadFilePath();
        QString selfUpdateDirPath = FileManager::GetSelfUpdateTempDirectory();
        //archive file scope. At the end of the scope file will be closed if necessary
        {
            QByteArray data = currentDownload->readAll();

            currentDownload->deleteLater();
            currentDownload = nullptr;
            //create an archive with a new version
            if (!FileManager::CreateFileAndWriteData(tempArchiveFilePath, data))
            {
                ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_UPDATE, tr("Can not create launcher archive in the Launcher folder"));
                ui->label->setText(tr("Launcher was not updated!"));
                return;
            }
        }

        //unpack new version
        ZipUtils::CompressedFilesAndSizes files;
        SelfUpdater_local::SelfUpdaterZipFunctor functor(ui->progressBar_processing);
        if ((ZipUtils::GetFileList(tempArchiveFilePath, files, functor)
             && ZipUtils::UnpackZipArchive(tempArchiveFilePath, selfUpdateDirPath, files, functor)) == false)
        {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            ui->label->setText(tr("Launcher was not updated!"));
            return;
        }

        FileManager::DeleteDirectory(FileManager::GetTempDirectory());
        QString tempDir = FileManager::GetTempDirectory(); //create temp directory
        //remove old launcher files except download folder, temp folder and update folder
        if (FileManager::MoveLauncherRecursively(appDirPath, tempDir)
            && FileManager::MoveLauncherRecursively(selfUpdateDirPath, appDirPath))
        {
            QStringList info(files.keys());
            QByteArray data = info.join('\n').toUtf8().data();
            if (!FileManager::CreateFileAndWriteData(FileManager::GetPackageInfoFilePath(), data))
            {
                ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_UPDATE, tr("Can not create information of launcher build!"));
                ui->label->setText(tr("Launcher was not updated!"));
                return;
            }

            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            ui->label->setText(tr("Launcher was updated!\nPlease relaunch application!"));
            connect(this, &QDialog::finished, qApp, &QApplication::quit);
        }
        else
        {
            QMessageBox::critical(this, tr("Critical error while updating launcher!"), tr("Critical error uccurred while updating launcher! Install application manually, please!"));
            reject();
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
