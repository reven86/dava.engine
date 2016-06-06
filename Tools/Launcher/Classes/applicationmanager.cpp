#include "applicationmanager.h"
#include "filemanager.h"
#include "errormessenger.h"
#include "processhelper.h"
#include <QFile>
#include <QDebug>
#include <QMessageBox>

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
{
    localConfigFilePath = FileManager::GetDocumentsDirectory() + LOCAL_CONFIG_NAME;
    LoadLocalConfig(localConfigFilePath);
}

void ApplicationManager::LoadLocalConfig(const QString& configPath)
{
    QFile configFile(configPath);
    localConfig.Clear();
    if (configFile.open(QFile::ReadWrite))
    {
        QByteArray data = configFile.readAll();
        configFile.close();
        localConfig.Parse(data);
        localConfig.UpdateApplicationsNames();
    }
    else
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_DOC_ACCESS);
    }
    //kostil
    Branch* branch = localConfig.GetBranch("ST");
    if (branch != nullptr)
    {
        branch->id = "Stable";
    }
    branch = localConfig.GetBranch("BLITZTOSTABLE");
    if (branch != nullptr)
    {
        branch->id = "Blitz To Stable";
    }
}

void ApplicationManager::ParseRemoteConfigData(const QByteArray& data)
{
    if (data.isEmpty())
    {
        return;
    }
    remoteConfig.Parse(data);
    QString webPageUrl = remoteConfig.GetWebpageURL();
    if (!webPageUrl.isEmpty())
    {
        localConfig.SetWebpageURL(webPageUrl);
    }
    localConfig.CopyStringsAndFavsFromConfig(remoteConfig);
    localConfig.SaveToFile(localConfigFilePath);
}

QString ApplicationManager::GetApplicationDirectory(const QString& branchID, const QString& appID, bool mustExists) const
{
    QString runPath = FileManager::GetApplicationDirectory(branchID, appID);
    if (QFile::exists(runPath))
    {
        return runPath;
    }
    QList<QString> branchKeys = localConfig.GetStrings().keys(branchID);
    branchKeys.append(branchID);
    for (const QString& branchKey : branchKeys)
    {
        QList<QString> appKeys = localConfig.GetStrings().keys(appID);
        appKeys.append(appID);
        for (const QString& appKey : appKeys)
        {
            QString newRunPath = FileManager::GetApplicationDirectory(branchKey, appKey);
            if (QFile::exists(newRunPath))
            {
                return newRunPath;
            }
        }
    }
    if (mustExists)
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_PATH, tr("Application path %1 %2 not exists!").arg(branchID).arg(appID));
        return "";
    }
    else
    {
        FileManager::MakeDirectory(runPath);
        return runPath;
    }
}

bool ApplicationManager::ShouldShowNews()
{
    return remoteConfig.GetNewsID() != localConfig.GetNewsID();
}

void ApplicationManager::NewsShowed()
{
    localConfig.SetLastNewsID(remoteConfig.GetNewsID());
}

void ApplicationManager::CheckUpdates(QQueue<UpdateTask>& tasks)
{
    //check self-update
    if (remoteConfig.GetLauncherVersion() != localConfig.GetLauncherVersion())
    {
        AppVersion version;
        version.id = remoteConfig.GetLauncherVersion();
        version.url = remoteConfig.GetLauncherURL();
        tasks.push_back(UpdateTask("", "", version, true));

        return;
    }

    //check applications update
    int branchCount = remoteConfig.GetBranchCount();
    for (int i = 0; i < branchCount; ++i)
    {
        Branch* branch = remoteConfig.GetBranch(i);
        if (!localConfig.GetBranch(branch->id))
            continue;

        int appCount = branch->GetAppCount();
        for (int j = 0; j < appCount; ++j)
        {
            Application* app = branch->GetApplication(j);
            if (!localConfig.GetApplication(branch->id, app->id))
                continue;

            if (app->GetVerionsCount() == 1)
            {
                AppVersion* appVersion = app->GetVersion(0);
                Application* localApp = localConfig.GetApplication(branch->id, app->id);
                AppVersion* localAppVersion = localApp->GetVersion(0);
                if (localAppVersion->id != appVersion->id)
                    tasks.push_back(UpdateTask(branch->id, app->id, *appVersion));
            }
        }
    }

    int localBranchCount = localConfig.GetBranchCount();
    for (int i = 0; i < localBranchCount; ++i)
    {
        Branch* branch = localConfig.GetBranch(i);
        if (!remoteConfig.GetBranch(branch->id))
            tasks.push_back(UpdateTask(branch->id, "", AppVersion(), false, true));
    }
}

void ApplicationManager::OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version)
{
    localConfig.InsertApplication(branchID, appID, version);
    localConfig.SaveToFile(localConfigFilePath);
}

QString ApplicationManager::GetString(const QString& stringID) const
{
    QString string = stringID;
    string = remoteConfig.GetString(stringID);
    if (string == stringID)
        string = localConfig.GetString(stringID);
    return string;
}

ConfigParser* ApplicationManager::GetRemoteConfig()
{
    return &remoteConfig;
}

ConfigParser* ApplicationManager::GetLocalConfig()
{
    return &localConfig;
}

void ApplicationManager::RunApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    AppVersion* version = localConfig.GetAppVersion(branchID, appID, versionID);
    if (version)
    {
        QString runPath = GetApplicationDirectory(branchID, appID) + version->runPath;
        if (runPath.isEmpty())
        {
            return;
        }
        if (!ProcessHelper::IsProcessRuning(runPath))
            ProcessHelper::RunProcess(runPath);
        else
            ErrorMessenger::ShowNotificationDlg("Application is already launched.");
    }
}

bool ApplicationManager::RemoveApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    AppVersion* version = localConfig.GetAppVersion(branchID, appID, versionID);
    if (version)
    {
        QString runPath = GetApplicationDirectory(branchID, appID) + version->runPath;
        if (runPath.isEmpty())
        {
            return false;
        }
        while (ProcessHelper::IsProcessRuning(runPath))
        {
            int result = ErrorMessenger::ShowRetryDlg(true);
            if (result == QMessageBox::Cancel)
                return false;
        }

        QString appPath = GetApplicationDirectory(branchID, appID);
        if (appPath.isEmpty())
        {
            return false;
        }
        FileManager::DeleteDirectory(appPath);
        localConfig.RemoveApplication(branchID, appID, versionID);
        localConfig.SaveToFile(localConfigFilePath);

        return true;
    }
    return false;
}

bool ApplicationManager::RemoveBranch(const QString& branchID)
{
    Branch* branch = localConfig.GetBranch(branchID);
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
            QString runPath = GetApplicationDirectory(branchID, app->id) + version->runPath;
            if (runPath.isEmpty())
            {
                return false;
            }
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
    localConfig.RemoveBranch(branch->id);
    localConfig.SaveToFile(localConfigFilePath);

    return true;
}
