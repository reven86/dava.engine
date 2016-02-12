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


#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "filemanager.h"
#include "applicationmanager.h"
#include "errormessanger.h"
#include "processhelper.h"
#include "mainwindow.h"
#include "defines.h"
#include <QNetworkReply>
#include <QDir>
#include <QPushButton>
#include <QListWidget>
#include <QDebug>
#include <QTreeView>

UpdateDialog::UpdateDialog(const QQueue<UpdateTask>& taskQueue, ApplicationManager* _appManager, QNetworkAccessManager* accessManager, QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint)
    , ui(new Ui::UpdateDialog)
    , unpacker(new ZipUnpacker())
    , networkManager(accessManager)
    , tasks(taskQueue)
    , appManager(_appManager)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->progressBar2->setValue(0);

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(OnCancelClicked()));
    connect(this, SIGNAL(UpdateDownloadProgress(int)), ui->progressBar, SLOT(setValue(int)));
    connect(this, SIGNAL(UpdateUnpackProgress(int)), ui->progressBar2, SLOT(setValue(int)));
    connect(unpacker.get(), SIGNAL(OnProgress(int, int)), this, SLOT(UnpackProgress(int, int)));
    connect(unpacker.get(), SIGNAL(OnError(int)), this, SLOT(UnpackError(int)));

    tasksCount = tasks.size();

    StartNextTask();
}

UpdateDialog::~UpdateDialog() = default;

void UpdateDialog::OnCancelClicked()
{
    if (currentDownload)
        currentDownload->abort();
    outputFile.close();

    FileManager::Instance()->ClearTempDirectory();

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
        ui->progressBar->setValue(0);
        ui->progressBar2->setValue(0);

        setWindowTitle(QString("Updating in progress... (%1/%2)").arg(tasksCount - tasks.size() + 1).arg(tasksCount));

        UpdateTask task = tasks.front();

        if (task.isRemoveBranch)
        {
            AddTopLogValue(QString("Removing branch %1:").arg(appManager->GetString(task.branchID)));

            if (appManager->RemoveBranch(task.branchID))
            {
                AddLogValue("Removing Complete!");
                CompleteLog();

                emit UpdateDownloadProgress(100);
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
            const QString& archiveFilepath = FileManager::Instance()->GetTempDownloadFilepath();

            outputFile.setFileName(archiveFilepath);
            outputFile.open(QFile::WriteOnly);
            currentDownload = networkManager->get(QNetworkRequest(QUrl(task.version.url)));

            connect(currentDownload, SIGNAL(finished()), this, SLOT(DownloadFinished()));
            connect(currentDownload, SIGNAL(readyRead()), this, SLOT(DownloadReadyRead()));
            connect(currentDownload, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(NetworkError(QNetworkReply::NetworkError)));
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

void UpdateDialog::NetworkError(QNetworkReply::NetworkError code)
{
    lastErrorDesrc = currentDownload->errorString();
    lastErrorCode = code;

    currentDownload->deleteLater();
    currentDownload = 0;
}

void UpdateDialog::DownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal)
    {
        int percentage = (static_cast<double>(bytesReceived) / bytesTotal) * 100;
        emit UpdateDownloadProgress(percentage);

        UpdateLastLogValue(QString("Downloading file...  %1%").arg(percentage));
    }
}

void UpdateDialog::DownloadFinished()
{
    QString filePath = outputFile.fileName();
    outputFile.close();

    const UpdateTask &task = tasks.head();

    if (currentDownload)
    {
        currentDownload->deleteLater();
        currentDownload = nullptr;

        QString appDir = FileManager::Instance()->GetApplicationFolder(task.branchID, task.appID);

        QString runPath = appDir + task.version.runPath;
        while (ProcessHelper::IsProcessRuning(runPath))
            ErrorMessanger::Instance()->ShowRetryDlg(false);

        FileManager::Instance()->DeleteDirectory(appDir);

        UpdateLastLogValue(QString("Download Complete!"));
        AddLogValue("Unpacking archive...");

        ui->cancelButton->setEnabled(false);

        if(unpacker->UnZipFile(filePath, appDir))
        {
            emit AppInstalled(task.branchID, task.appID, task.version);
            UpdateLastLogValue("Unpack Complite!");
            CompleteLog();
        }
        else
        {
            UpdateLastLogValue("Unpack Fail!");
            BreakLog();
        }
        ui->cancelButton->setEnabled(true);
        FileManager::Instance()->ClearTempDirectory();
    }
    else if (lastErrorCode != QNetworkReply::OperationCanceledError)
    {
        UpdateLastLogValue("Download Fail!");
        BreakLog();

        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_NETWORK, lastErrorCode, lastErrorDesrc);
    }
    tasks.dequeue();
    StartNextTask();

}

void UpdateDialog::UnpackProgress(int current, int count)
{
    if (count)
    {
        int percentage = ((double)current / count) * 100;
        emit UpdateUnpackProgress(percentage);

        UpdateLastLogValue(QString("Unpacking archive...  %1%").arg(percentage));
    }
}

void UpdateDialog::UnpackError(int code)
{
    ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_UNPACK, code, unpacker->GetErrorString(code));
}

void UpdateDialog::DownloadReadyRead()
{
    outputFile.write(currentDownload->readAll());
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
    }
    if (currentLogItem != nullptr)
    {
        currentLogItem->setTextColor(0, LOG_COLOR_FAIL);
    }
    currentTopLogItem = nullptr;
    currentLogItem = nullptr;
}

void UpdateDialog::CompleteLog()
{
    if (currentTopLogItem != nullptr)
    {
        currentTopLogItem->setTextColor(0, LOG_COLOR_COMPLETE);
    }
    if (currentLogItem != nullptr)
    {
        currentLogItem->setTextColor(0, LOG_COLOR_COMPLETE);
    }
    currentTopLogItem = nullptr;
    currentLogItem = nullptr;
}
