#include "applicationmanager.h"
#include "filemanager.h"
#include "errormessenger.h"
#include "processhelper.h"
#include "filemanager.h"

#include "QtHelpers/HelperFunctions.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>

namespace ApplicationManagerDetails
{

QString RemoveWhitespace(const QString& str)
{
    QString replacedStr = str;
    QRegularExpression spaceRegex("\\s+");
    replacedStr.replace(spaceRegex, "");
    return replacedStr;
}
}

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
    , fileManager(new FileManager(this))
{
    localConfigFilePath = FileManager::GetDocumentsDirectory() + LOCAL_CONFIG_NAME;
    LoadLocalConfig(localConfigFilePath);
}

void ApplicationManager::LoadLocalConfig(const QString& configPath)
{
    QFile configFile(configPath);
    if (!configFile.exists())
    {
        return;
    }
    localConfig.Clear();
    if (configFile.open(QFile::ReadOnly))
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
    SaveLocalConfig();
}

void ApplicationManager::SaveLocalConfig() const
{
    localConfig.SaveToFile(localConfigFilePath);
}

QString ApplicationManager::GetApplicationDirectory_kostil(const QString& branchID, const QString& appID) const
{
    QString path = fileManager->GetBaseAppsDirectory() + branchID + "/" + appID + "/";
    return path;
}

QString ApplicationManager::GetApplicationDirectory(QString branchID, QString appID, bool mustExists) const
{
    branchID = ApplicationManagerDetails::RemoveWhitespace(branchID);
    appID = ApplicationManagerDetails::RemoveWhitespace(appID);

    //try to get right path
    QString runPath = fileManager->GetApplicationDirectory(branchID, appID);
    if (QFile::exists(runPath))
    {
        return runPath;
    }

    //try to get old ugly path with a bug on "/" symbol
    QString tmpRunPath = GetApplicationDirectory_kostil(branchID, appID);
    if (QFile::exists(tmpRunPath))
    {
        return tmpRunPath;
    }
    //we can have old branche name or old app name
    QList<QString> branchKeys = localConfig.GetStrings().keys(branchID);
    //it can be combination of old and new names
    branchKeys.append(branchID);
    for (const QString& branchKey : branchKeys)
    {
        QList<QString> appKeys = localConfig.GetStrings().keys(appID);
        appKeys.append(appID);
        for (const QString& appKey : appKeys)
        {
            QString newRunPath = fileManager->GetApplicationDirectory(branchKey, appKey);
            if (QFile::exists(newRunPath))
            {
                return newRunPath;
            }
            newRunPath = GetApplicationDirectory_kostil(branchKey, appKey);
            if (QFile::exists(newRunPath))
            {
                return newRunPath;
            }
        }
    }
    //we expect that folder exists
    //or we just downloaded it and did not find original folder? make new folder with a correct name
    if (mustExists)
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_PATH, tr("Application %1 in branch %2 not exists!").arg(appID).arg(branchID));
        return "";
    }
    else
    {
        FileManager::MakeDirectory(runPath);
        return runPath;
    }
}

FileManager* ApplicationManager::GetFileManager() const
{
    return fileManager;
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
    SaveLocalConfig();
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

QString ApplicationManager::ExtractApplicationRunPath(const QString& branchID, const QString& appID, const QString& versionID)
{
    AppVersion* version = localConfig.GetAppVersion(branchID, appID, versionID);
    if (version == nullptr)
    {
        return "";
    }
    QString runPath = GetApplicationDirectory(branchID, appID);
    QString localAppPath = version->runPath;
    if (localAppPath.isEmpty())
    {
        QString localAppID = ApplicationManagerDetails::RemoveWhitespace(appID);
#ifdef Q_OS_WIN
        localAppPath = localAppID + ".exe";
#elif defined(Q_OS_MAC)
        localAppPath = "ToolSet.app/Contents/MacOS/" + localAppID;
#else
#error "unsupported platform"
#endif //platform
    }
    runPath += localAppPath;
    if (!QFile::exists(runPath))
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_PATH, tr("application not found\n%1").arg(runPath));
        return "";
    }
    return runPath;
}

void ApplicationManager::ShowApplicataionInExplorer(const QString& branchID, const QString& appID, const QString& versionID)
{
    QString runPath = ExtractApplicationRunPath(branchID, appID, versionID);
    if (runPath.isEmpty())
    {
        return;
    }
    QtHelpers::ShowInOSFileManager(runPath);
}

void ApplicationManager::RunApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    QString runPath = ExtractApplicationRunPath(branchID, appID, versionID);
    if (runPath.isEmpty())
    {
        return;
    }
    if (!ProcessHelper::IsProcessRuning(runPath))
        ProcessHelper::RunProcess(runPath);
    else
        ErrorMessenger::ShowNotificationDlg("Application is already launched.");
}

bool ApplicationManager::RemoveApplication(const QString& branchID, const QString& appID, const QString& versionID)
{
    AppVersion* version = localConfig.GetAppVersion(branchID, appID, versionID);
    if (version)
    {
        QString runPath = GetApplicationDirectory(branchID, appID, false) + version->runPath;
        if (!runPath.isEmpty())
        {
            while (ProcessHelper::IsProcessRuning(runPath))
            {
                int result = ErrorMessenger::ShowRetryDlg(true);
                if (result == QMessageBox::Cancel)
                    return false;
            }
        }

        QString appPath = GetApplicationDirectory(branchID, appID);
        if (appPath.isEmpty())
        {
            return true;
        }
        FileManager::DeleteDirectory(appPath);
        localConfig.RemoveApplication(branchID, appID, versionID);
        SaveLocalConfig();
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

    QString branchPath = fileManager->GetBranchDirectory(branchID);
    FileManager::DeleteDirectory(branchPath);
    localConfig.RemoveBranch(branch->id);
    SaveLocalConfig();
    return true;
}
