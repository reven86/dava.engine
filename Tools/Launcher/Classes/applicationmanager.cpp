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


#include "applicationmanager.h"
#include "filemanager.h"
#include "errormessanger.h"
#include "processhelper.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>

ApplicationManager::ApplicationManager(QObject *parent) :
    QObject(parent),
    localConfig(0),
    remoteConfig(0)
{
    localConfigFilePath = FileManager::Instance()->GetDocumentsDirectory() + LOCAL_CONFIG_NAME;
    LoadLocalConfig(localConfigFilePath);
}

ApplicationManager::~ApplicationManager()
{
    SafeDelete(localConfig);
    SafeDelete(remoteConfig);
}

void ApplicationManager::LoadLocalConfig(const QString & configPath)
{
    QFile configFile(configPath);
    if(configFile.open(QFile::ReadWrite))
    {
        QByteArray data = configFile.readAll();
        configFile.close();
        localConfig = new ConfigParser(data);
    }
    else
    {
        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_DOC_ACCESS);
        localConfig = new ConfigParser(QByteArray());
    }
}

void ApplicationManager::ParseRemoteConfigData(const QByteArray & data)
{
    if(data.size())
    {
        SafeDelete(remoteConfig);
        remoteConfig = new ConfigParser(data);

        QString webPageUrl = remoteConfig->GetWebpageURL();
        if(!webPageUrl.isEmpty())
        {
            localConfig->SetWebpageURL(webPageUrl);
        }
        localConfig->CopyStringsAndFavsFromConfig(*remoteConfig);
        localConfig->SaveToYamlFile(localConfigFilePath);
    }
}

bool ApplicationManager::ShouldShowNews()
{
    return remoteConfig && remoteConfig->GetNewsID() != localConfig->GetNewsID();
}

void ApplicationManager::NewsShowed()
{
    if(remoteConfig)
        localConfig->SetLastNewsID(remoteConfig->GetNewsID());
}

void ApplicationManager::CheckUpdates(QQueue<UpdateTask> & tasks)
{
    //check self-update
    if(remoteConfig && remoteConfig->GetLauncherVersion() != localConfig->GetLauncherVersion())
    {
        AppVersion version;
        version.id = remoteConfig->GetLauncherVersion();
        version.url = remoteConfig->GetLauncherURL();
        tasks.push_back(UpdateTask("", "", version, true));

        return;
    }

    //check applications update
    if(remoteConfig)
    {
        int branchCount = remoteConfig->GetBranchCount();
        for(int i = 0; i < branchCount; ++i)
        {
            Branch * branch = remoteConfig->GetBranch(i);
            if(!localConfig->GetBranch(branch->id))
                continue;

            int appCount = branch->GetAppCount();
            for(int j = 0; j < appCount; ++j)
            {
                Application * app = branch->GetApplication(j);
                if(!localConfig->GetApplication(branch->id, app->id))
                    continue;

                if(app->GetVerionsCount() == 1)
                {
                    AppVersion * appVersion = app->GetVersion(0);
                    Application * localApp = localConfig->GetApplication(branch->id, app->id);
                    if(localApp->GetVersion(0)->id != appVersion->id)
                        tasks.push_back(UpdateTask(branch->id, app->id, *appVersion));
                }
            }
        }

        int localBranchCount = localConfig->GetBranchCount();
        for(int i = 0; i < localBranchCount; ++i)
        {
            Branch * branch = localConfig->GetBranch(i);
            if(!remoteConfig->GetBranch(branch->id))
                tasks.push_back(UpdateTask(branch->id, "", AppVersion(), false, true));
        }
    }
}

void ApplicationManager::OnAppInstalled(const QString & branchID, const QString & appID, const AppVersion & version)
{
    localConfig->InsertApplication(branchID, appID, version);
    localConfig->SaveToYamlFile(localConfigFilePath);
}

QString ApplicationManager::GetString(const QString & stringID) const
{
    QString string = stringID;
    if(remoteConfig)
        string = remoteConfig->GetString(stringID);
    if(localConfig && string == stringID)
        string = localConfig->GetString(stringID);
    return string;
}

ConfigParser * ApplicationManager::GetRemoteConfig()
{
    return remoteConfig;
}

ConfigParser * ApplicationManager::GetLocalConfig()
{
    return localConfig;
}

void ApplicationManager::RunApplication(const QString & branchID, const QString & appID, const QString & versionID)
{
    AppVersion * version = localConfig->GetAppVersion(branchID, appID, versionID);
    if(version)
    {
        QString runPath = FileManager::Instance()->GetApplicationFolder(branchID, appID) + version->runPath;
        if(!ProcessHelper::IsProcessRuning(runPath))
            ProcessHelper::RunProcess(runPath);
        else
            ErrorMessanger::Instance()->ShowNotificationDlg("Application is already launched.");
    }
}

bool ApplicationManager::RemoveApplication(const QString & branchID, const QString & appID, const QString & versionID)
{
    AppVersion * version = localConfig->GetAppVersion(branchID, appID, versionID);
    if(version)
    {
        QString runPath = FileManager::Instance()->GetApplicationFolder(branchID, appID) + version->runPath;
        while(ProcessHelper::IsProcessRuning(runPath))
        {
            int result = ErrorMessanger::Instance()->ShowRetryDlg(true);
            if(result == QMessageBox::Cancel)
                return false;
        }

        QString appPath = FileManager::Instance()->GetApplicationFolder(branchID, appID);
        FileManager::Instance()->DeleteDirectory(appPath);
        localConfig->RemoveApplication(branchID, appID, versionID);
        localConfig->SaveToYamlFile(localConfigFilePath);

        return true;
    }
    return false;
}

bool ApplicationManager::RemoveBranch(const QString & branchID)
{
    Branch * branch = localConfig->GetBranch(branchID);
    if(!branch)
        return false;

    int appCount = branch->GetAppCount();
    for(int i = 0; i < appCount; ++i)
    {
        Application * app = branch->GetApplication(i);
        int versionCount = app->GetVerionsCount();
        for(int j = 0; j < versionCount; ++j)
        {
            AppVersion * version = app->GetVersion(j);
            QString runPath = FileManager::Instance()->GetApplicationFolder(branchID, app->id) + version->runPath;
            while(ProcessHelper::IsProcessRuning(runPath))
            {
                int result = ErrorMessanger::Instance()->ShowRetryDlg(true);
                if(result == QMessageBox::Cancel)
                    return false;
            }
        }
    }

    QString branchPath = FileManager::Instance()->GetBranchFolder(branchID);
    FileManager::Instance()->DeleteDirectory(branchPath);
    localConfig->RemoveBranch(branch->id);
    localConfig->SaveToYamlFile(localConfigFilePath);

    return true;
}
