#include "applicationmanager.h"
#include "filemanager.h"
#include "errormessenger.h"
#include "processhelper.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
    , localConfig(0)
    , remoteConfig(new ConfigParser())
{
    localConfigFilePath = FileManager::GetDocumentsDirectory() + LOCAL_CONFIG_NAME;
    LoadLocalConfig(localConfigFilePath);
}

ApplicationManager::~ApplicationManager()
{
    SafeDelete(localConfig);
    SafeDelete(remoteConfig);
}

void ApplicationManager::LoadLocalConfig(const QString& configPath)
{
    QFile configFile(configPath);
    if (configFile.open(QFile::ReadWrite))
    {
        QByteArray data = configFile.readAll();
        configFile.close();
        localConfig = new ConfigParser();
        localConfig->Parse(data);
        localConfig->UpdateApplicationsNames();
    }
    else
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_DOC_ACCESS);
        localConfig = new ConfigParser();
    }
}

void ApplicationManager::ParseRemoteConfigData(const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }
    remoteConfig->Parse(data);
    QString webPageUrl = remoteConfig->GetWebpageURL();
    if (!webPageUrl.isEmpty())
    {
        localConfig->SetWebpageURL(webPageUrl);
    }
    localConfig->CopyStringsAndFavsFromConfig(*remoteConfig);
    localConfig->SaveToFile(localConfigFilePath);
}

bool ApplicationManager::ShouldShowNews()
{
    return remoteConfig && remoteConfig->GetNewsID() != localConfig->GetNewsID();
}

void ApplicationManager::NewsShowed()
{
    if (remoteConfig)
        localConfig->SetLastNewsID(remoteConfig->GetNewsID());
}

void ApplicationManager::CheckUpdates(QQueue<UpdateTask>& tasks)
{
    if (!remoteConfig)
    {
        return;
    }
    //check self-update
    if (remoteConfig->GetLauncherVersion() != localConfig->GetLauncherVersion())
    {
        AppVersion version;
        version.id = remoteConfig->GetLauncherVersion();
        version.url = remoteConfig->GetLauncherURL();
        tasks.push_back(UpdateTask("", "", version, true));

        return;
    }

    //check applications update
    int branchCount = remoteConfig->GetBranchCount();
    for (int i = 0; i < branchCount; ++i)
    {
        Branch* branch = remoteConfig->GetBranch(i);
        if (!localConfig->GetBranch(branch->id))
            continue;

        int appCount = branch->GetAppCount();
        for (int j = 0; j < appCount; ++j)
        {
            Application* app = branch->GetApplication(j);
            if (!localConfig->GetApplication(branch->id, app->id))
                continue;

            if (app->GetVerionsCount() == 1)
            {
                AppVersion* appVersion = app->GetVersion(0);
                Application* localApp = localConfig->GetApplication(branch->id, app->id);
                if (localApp->GetVersion(0)->id != appVersion->id)
                    tasks.push_back(UpdateTask(branch->id, app->id, *appVersion));
            }
        }
    }

    int localBranchCount = localConfig->GetBranchCount();
    for (int i = 0; i < localBranchCount; ++i)
    {
        Branch* branch = localConfig->GetBranch(i);
        if (!remoteConfig->GetBranch(branch->id))
            tasks.push_back(UpdateTask(branch->id, "", AppVersion(), false, true));
    }
}

void ApplicationManager::OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version)
{
    localConfig->InsertApplication(branchID, appID, version);
    localConfig->SaveToFile(localConfigFilePath);
}

QString ApplicationManager::GetString(const QString& stringID) const
{
    QString string = stringID;
    if (remoteConfig)
        string = remoteConfig->GetString(stringID);
    if (localConfig && string == stringID)
        string = localConfig->GetString(stringID);
    return string;
}

ConfigParser* ApplicationManager::GetRemoteConfig() const
{
    return remoteConfig;
}

ConfigParser* ApplicationManager::GetLocalConfig() const
{
    return localConfig;
}

void ApplicationManager::RunApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    AppVersion* version = localConfig->GetAppVersion(branchID, appID, versionID);
    if (version)
    {
        QString runPath = FileManager::GetApplicationDirectory(branchID, appID) + version->runPath;
        if (!ProcessHelper::IsProcessRuning(runPath))
            ProcessHelper::RunProcess(runPath);
        else
            ErrorMessenger::ShowNotificationDlg("Application is already launched.");
    }
}

bool ApplicationManager::RemoveApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    AppVersion* version = localConfig->GetAppVersion(branchID, appID, versionID);
    if (version)
    {
        QString runPath = FileManager::GetApplicationDirectory(branchID, appID) + version->runPath;
        while (ProcessHelper::IsProcessRuning(runPath))
        {
            int result = ErrorMessenger::ShowRetryDlg(true);
            if (result == QMessageBox::Cancel)
                return false;
        }

        QString appPath = FileManager::GetApplicationDirectory(branchID, appID);
        FileManager::DeleteDirectory(appPath);
        localConfig->RemoveApplication(branchID, appID, versionID);
        localConfig->SaveToFile(localConfigFilePath);

        return true;
    }
    return false;
}

bool ApplicationManager::RemoveBranch(const QString& branchID)
{
    Branch* branch = localConfig->GetBranch(branchID);
    if (!branch)
        return false;

    int appCount = branch->GetAppCount();
    for (int i = 0; i < appCount; ++i)
    {
        Application* app = branch->GetApplication(i);
        int versionCount = app->GetVerionsCount();
        for (int j = 0; j < versionCount; ++j)
        {
            AppVersion* version = app->GetVersion(j);
            QString runPath = FileManager::GetApplicationDirectory(branchID, app->id) + version->runPath;
            while (ProcessHelper::IsProcessRuning(runPath))
            {
                int result = ErrorMessenger::ShowRetryDlg(true);
                if (result == QMessageBox::Cancel)
                    return false;
            }
        }
    }

    QString branchPath = FileManager::GetBranchDirectory(branchID);
    FileManager::DeleteDirectory(branchPath);
    localConfig->RemoveBranch(branch->id);
    localConfig->SaveToFile(localConfigFilePath);

    return true;
}
