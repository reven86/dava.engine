#include "applicationmanager.h"
#include "filemanager.h"
#include "errormessenger.h"
#include "filemanager.h"

#include "QtHelpers/HelperFunctions.h"
#include "QtHelpers/ProcessCommunication.h"
#include "QtHelpers/ProcessHelper.h"

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QThread>

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
    , processCommunication(new ProcessCommunication(this))

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

AppVersion* ApplicationManager::GetInstalledVersion(const QString& branchID, const QString& appID)
{
    Application* app = localConfig.GetApplication(branchID, appID);
    if (app == nullptr)
    {
        return nullptr;
    }
    AppVersion* version = app->GetVersion(0);
    return version;
}

bool ApplicationManager::TryStopApp(const QString& runPath) const
{
    ProcessCommunication::eReply reply = processCommunication->SendSync(ProcessCommunication::eMessage::QUIT, runPath);
    if (reply == ProcessCommunication::eReply::ACCEPT)
    {
        QElapsedTimer timer;
        timer.start();
        const int maxWaitTime = 10000; //10 secs;
        bool isStillRunning = true;
        while (timer.elapsed() < maxWaitTime && isStillRunning)
        {
            isStillRunning = ProcessHelper::IsProcessRuning(runPath);
            QThread::msleep(100);
        }
        return isStillRunning == false;
    }
    else
    {
        ErrorMessenger::LogMessage(QtWarningMsg, tr("Can not stop ACS, answer is %1").arg(processCommunication->GetReplyString(reply)));
    }
    return false;
}

bool ApplicationManager::RemoveApplicationImpl(const QString& branchID, const QString& appID)
{
    AppVersion* version = GetInstalledVersion(branchID, appID);
    if (version == nullptr)
    {
        return true;
    }
    QString appDirPath = GetApplicationDirectory(branchID, appID, version->isToolSet, false);
    bool success = FileManager::DeleteDirectory(appDirPath);
    if (success)
    {
        localConfig.RemoveApplication(branchID, appID, version->id);
        SaveLocalConfig();
    }
    else
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_FILE, "Can not remove directory " + appDirPath + "\nApplication need to be reinstalled!\nIf this error occurs again - remove directory by yourself, please");
    }
    return success;
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

QString ApplicationManager::GetApplicationDirectory(QString branchID, QString appID, bool isToolSet, bool mustExists) const
{
    appID = isToolSet ? "Toolset" : appID;

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
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_PATH, tr("Application %1 in branch %2 does not exists!").arg(appID).arg(branchID));
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

QString ApplicationManager::GetLocalAppPath(const AppVersion* version, const QString& appID)
{
    Q_ASSERT(!appID.isEmpty());
    if (version == nullptr)
    {
        return QString();
    }
    QString runPath = version->runPath;
    if (runPath.isEmpty())
    {
        QString correctID = ApplicationManagerDetails::RemoveWhitespace(appID);
#ifdef Q_OS_WIN
        runPath = correctID + ".exe";
#elif defined(Q_OS_MAC)
        runPath = correctID + ".app";
#else
#error "unsupported platform"
#endif //platform
    }
    return runPath;
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
        AppVersion newVersion;
        newVersion.id = remoteConfig.GetLauncherVersion();
        newVersion.url = remoteConfig.GetLauncherURL();

        tasks.push_back(UpdateTask("", "", nullptr, newVersion, true));

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
                    tasks.push_back(UpdateTask(branch->id, app->id, localAppVersion, *appVersion));
            }
        }
    }

    int localBranchCount = localConfig.GetBranchCount();
    for (int i = 0; i < localBranchCount; ++i)
    {
        Branch* branch = localConfig.GetBranch(i);
        if (!remoteConfig.GetBranch(branch->id))
            tasks.push_back(UpdateTask(branch->id, "", nullptr, AppVersion(), false, true));
    }
}

void ApplicationManager::OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version)
{
    localConfig.InsertApplication(branchID, appID, version);
    localConfig.UpdateApplicationsNames();
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

ProcessCommunication* ApplicationManager::GetProcessCommunicationModule() const
{
    return processCommunication;
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
    QString runPath = GetApplicationDirectory(branchID, appID, version->isToolSet);
    QString localAppPath = GetLocalAppPath(version, appID);
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

void ApplicationManager::RunApplication(const QString& branchID, const QString& appID)
{
    AppVersion* installedAppVersion = GetInstalledVersion(branchID, appID);
    if (installedAppVersion == nullptr)
    {
        return;
    }
    RunApplication(branchID, appID, installedAppVersion->id);
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

bool ApplicationManager::RemoveApplication(const QString& branchID, const QString& appID, bool canReject)
{
    Application* app = localConfig.GetApplication(branchID, appID);
    if (app == nullptr)
    {
        return true;
    }
    AppVersion* version = app->GetVersion(0);
    if (version == nullptr)
    {
        return true;
    }

    auto canRemoveApp = [this, canReject](const QString& branchID, const QString& appID) -> bool {
        AppVersion* version = GetInstalledVersion(branchID, appID);
        if (version == nullptr)
        {
            return true;
        }
        QString appDirPath = GetApplicationDirectory(branchID, appID, version->isToolSet, false);
        if (appDirPath.isEmpty())
        {
            return true;
        }
        QString runPath = appDirPath + GetLocalAppPath(version, appID);
        bool triedToStopProgrammly = false;
        while (ProcessHelper::IsProcessRuning(runPath))
        {
            if (triedToStopProgrammly == false && appID.contains("assetcacheserver", Qt::CaseInsensitive))
            {
                if (TryStopApp(runPath))
                {
                    return true;
                }
                else
                {
                    triedToStopProgrammly = true;
                }
            }
            int result = ErrorMessenger::ShowRetryDlg(appID, runPath, canReject);
            if (result == QMessageBox::Cancel)
            {
                return false;
            }
        }
        return true;
    };

    if (version->isToolSet)
    {
        //check that we can remove folder "toolset" first
        for (const QString& fakeAppID : localConfig.GetTranslatedToolsetApplications())
        {
            if (canRemoveApp(branchID, fakeAppID) == false)
            {
                ErrorMessenger::LogMessage(QtWarningMsg, "Can not remove application " + fakeAppID + " from branch " + branchID);
                return false;
            }
        }
        //remove folder from hardDisk and from config parser
        //right now we call DeleteDirectory three times for one folder. This is required by design of ApplicationManager and ConfigParser classes
        for (const QString& fakeAppID : localConfig.GetTranslatedToolsetApplications())
        {
            if (RemoveApplicationImpl(branchID, fakeAppID) == false)
            {
                return false;
            }
        }
    }
    else
    {
        if (canRemoveApp(branchID, appID) == false)
        {
            ErrorMessenger::LogMessage(QtWarningMsg, "Can not remove application " + appID + " from branch " + branchID);
            return false;
        }
        return RemoveApplicationImpl(branchID, appID);
    }
    return true;
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
            QString runPath = GetApplicationDirectory(branchID, app->id, version->isToolSet, true) + GetLocalAppPath(version, app->id);
            while (ProcessHelper::IsProcessRuning(runPath))
            {
                int result = ErrorMessenger::ShowRetryDlg(app->id, runPath, true);
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

bool ApplicationManager::PrepareToInstallNewApplication(const QString& branchID, const QString& appID, bool willInstallToolset, QStringList& appsToRestart)
{
    //we need to remove all toolset applications, whether they was installed or not
    auto needRestartLater = [this](const QString& branchID, const QString& appName) {
        if (appName.contains("assetcacheserver", Qt::CaseInsensitive) == false)
        {
            return false;
        }
        AppVersion* version = GetInstalledVersion(branchID, appName);
        if (version == nullptr)
        {
            return false;
        }
        QString path = GetApplicationDirectory(branchID, appName, version->isToolSet, false) + GetLocalAppPath(version, appName);
        return ProcessHelper::IsProcessRuning(path);
    };
    if (willInstallToolset)
    {
        QStringList toolsetApps = localConfig.GetTranslatedToolsetApplications();
        for (auto iter = toolsetApps.begin(); iter != toolsetApps.end(); ++iter)
        {
            QString appName = *iter;
            if (needRestartLater(branchID, appName))
            {
                appsToRestart.push_back(appName);
            }
            if (false == RemoveApplication(branchID, appName, false))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        if (needRestartLater(branchID, appID))
        {
            appsToRestart.push_back(appID);
        }
        //if current application is a part of toolset, all toolset applications will be removed.
        return RemoveApplication(branchID, appID, false);
    }
}
