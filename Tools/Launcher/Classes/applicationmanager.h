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


#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include "configparser.h"
#include "defines.h"
#include "updatedialog.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

class ConfigDownloader;
class ApplicationManager: public QObject
{
    Q_OBJECT
public:
    explicit ApplicationManager(QObject *parent = 0);
    ~ApplicationManager();

    void RefreshRemoteConfig();

    QString GetString(const QString & stringID) const;

    ConfigParser * GetLocalConfig();
    ConfigParser * GetRemoteConfig();

    void CheckUpdates(QQueue<UpdateTask> & tasks);

    void RunApplication(const QString & branchID, const QString & appID, const QString & versionID);
    bool RemoveApplication(const QString & branchID, const QString & appID, const QString & versionID);
    bool RemoveBranch(const QString & branchID);

    bool ShouldShowNews();
    void NewsShowed();

signals:
    void Refresh();

public slots:
    void OnAppInstalled(const QString & branchID, const QString & appID, const AppVersion & version);

private:
    void LoadLocalConfig(const QString & configPath);
    void ParseRemoteConfigData(const QByteArray & data);

    QString localConfigFilePath;

    ConfigParser * localConfig;
    ConfigParser * remoteConfig;

    friend class ConfigDownloader;
};

#endif // APPLICATIONMANAGER_H
