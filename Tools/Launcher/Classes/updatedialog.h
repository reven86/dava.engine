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


#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include "configparser.h"
#include "zipunpacker.h"
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QFile>
#include <QTreeWidgetItem>

namespace Ui {
class UpdateDialog;
}

class ApplicationManager;

struct UpdateTask
{
    UpdateTask(const QString & branch, const QString & app, const AppVersion & _version, bool _isSelfUpdate = false, bool _isRemove = false) :
        branchID(branch), appID(app), version(_version), isSelfUpdate(_isSelfUpdate), isRemoveBranch(_isRemove) {}

    QString branchID;
    QString appID;
    AppVersion version;
    bool isSelfUpdate;
    bool isRemoveBranch;
};

class UpdateDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UpdateDialog(const QQueue<UpdateTask> & taskQueue, ApplicationManager * _appManager, QNetworkAccessManager * accessManager, QWidget *parent = 0);
    ~UpdateDialog();

signals:
    void UpdateDownloadProgress(int value);
    void UpdateUnpackProgress(int value);
    void AppInstalled(const QString & branchID, const QString & appID, const AppVersion & version);

public slots:
    void OnCancelClicked();
    void UnpackProgress(int, int);
    void UnpackComplete();
    void UnpackError(int);

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void DownloadFinished();
    void DownloadReadyRead();
    void StartNextTask();

private:
    void UpdateButton();

    void AddTopLogValue(const QString & log);
    void AddLogValue(const QString & log);
    void UpdateLastLogValue(const QString & log);
    void BreakLog();
    void CompleteLog();

    Ui::UpdateDialog *ui;

    QNetworkAccessManager * networkManager;
    QNetworkReply * currentDownload;

    int tasksCount;

    QFile outputFile;
    QQueue<UpdateTask> tasks;

    QTreeWidgetItem * currentLogItem;
    QTreeWidgetItem * currentTopLogItem;

    ZipUnpacker * unpacker;

    int lastErrorCode;
    QString lastErrorDesrc;

    ApplicationManager * appManager;
};

#endif // UPDATEDIALOG_H
