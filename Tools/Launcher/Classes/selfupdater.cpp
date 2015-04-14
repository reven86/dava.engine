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
#include "processhelper.h"
#include "errormessanger.h"
#include <QProcess>

SelfUpdater::SelfUpdater(const QString & arcUrl, QNetworkAccessManager * accessManager, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::SelfUpdater),
    archiveUrl(arcUrl),
    networkManager(accessManager),
    currentDownload(0),
    unpacker(0),
    lastErrorCode(0)
{
    ui->setupUi(this);

    unpacker = new ZipUnpacker();

    connect(this, SIGNAL(StartUpdating()), this, SLOT(OnStartUpdating()));

    emit StartUpdating();
}

SelfUpdater::~SelfUpdater()
{
    SafeDelete(ui);
    SafeDelete(unpacker);
}

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
    currentDownload = 0;
}

void SelfUpdater::DownloadFinished()
{
    if(currentDownload)
    {
        FileManager::Instance()->ClearTempDirectory();

        const QString & archiveFilePath = FileManager::Instance()->GetTempDownloadFilepath();
        const QString & tempDir = FileManager::Instance()->GetTempDirectory();
        const QString & appDir = FileManager::Instance()->GetLauncherDirectory();
        const QString & selfUpdateDir = FileManager::Instance()->GetSelfUpdateTempDirectory();

        QFile archiveFile(archiveFilePath);
        archiveFile.open(QFile::WriteOnly);
        archiveFile.write(currentDownload->readAll());
        archiveFile.close();

        currentDownload->deleteLater();
        currentDownload = 0;

        unpacker->UnZipFile(archiveFilePath, selfUpdateDir);

        FileManager::Instance()->MoveFilesOnlyToDirectory(appDir, tempDir);
        FileManager::Instance()->MoveFilesOnlyToDirectory(selfUpdateDir, appDir);
        FileManager::Instance()->DeleteDirectory(selfUpdateDir);

        ErrorMessanger::Instance()->ShowNotificationDlg("Launcher was updated. Please, relaunch application.");

        qApp->exit();
    }
    else if(lastErrorCode != QNetworkReply::OperationCanceledError)
    {
        setResult(QDialog::Rejected);
        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_NETWORK, lastErrorCode, lastErrorDesrc);
        close();
    }
}
