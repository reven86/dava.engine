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


#include "configdownloader.h"
#include "ui_configdownloader.h"
#include "filemanager.h"
#include "processhelper.h"
#include "errormessenger.h"
#include "applicationmanager.h"
#include <QProcess>

ConfigDownloader::ConfigDownloader(ApplicationManager* manager, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::ConfigDownloader)
    , appManager(manager)
{
    ui->setupUi(this);
}

ConfigDownloader::~ConfigDownloader()
{
    SafeDelete(ui);
}

int ConfigDownloader::exec()
{
    CreateDownloaderAndLaunch(QUrl("http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=builds"));
    CreateDownloaderAndLaunch(QUrl("http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=branches "));
    CreateDownloaderAndLaunch(QUrl("http://ba-manager.wargaming.net/panel/modules/json_lite.php?source=launcher"));
    return QDialog::exec();
}

void ConfigDownloader::DownloadFinished(QByteArray downloadedData, QList<QPair<QByteArray, QByteArray>> rawHeaderList, int errorCode, QString errorDescr)
{
    if (errorCode)
    {
        if (errorCode != QNetworkReply::OperationCanceledError)
        {
            ErrorMessenger::Instance()->ShowErrorMessage(ErrorMessenger::ERROR_NETWORK, errorCode, errorDescr);
        }
        reject();
    }
    else
    {
        const QByteArray contentTypeConst("Content-Type");
        for (QList<QPair<QByteArray, QByteArray>>::ConstIterator it = rawHeaderList.begin(); it != rawHeaderList.end(); ++it)
        {
            const QPair<QByteArray, QByteArray>& pair = *it;
            if (pair.first == contentTypeConst)
            {
                //appManager->ParseRemoteConfigData(downloadedData);
            }
        }

        accept();
    }
}

void ConfigDownloader::CreateDownloaderAndLaunch(const QUrl& url)
{
    FileDownloader* downloader = new FileDownloader(this);
    connect(ui->cancelButton, &QPushButton::clicked, downloader, &FileDownloader::Cancel);
    connect(downloader, &FileDownloader::Finished, this, &ConfigDownloader::DownloadFinished);

    downloader->Download(url);
}
