#include "Core/ApplicationManager.h"
#include "Core/BAManagerClient.h"
#include "Core/ConfigRefresher.h"
#include "Core/UrlsHolder.h"

#include "Core/Tasks/RunApplicationTask.h"
#include "Core/Tasks/RemoveApplicationTask.h"
#include "Core/Tasks/SelfUpdateTask.h"
#include "Core/Tasks/LoadLocalConfigTask.h"
#include "Core/Tasks/UpdateConfigTask.h"

#include "Gui/PreferencesDialog.h"

#include "Utils/AppsCommandsSender.h"
#include "Utils/FileManager.h"
#include "Utils/ErrorMessenger.h"

#include <QtHelpers/HelperFunctions.h>
#include <QtHelpers/ProcessHelper.h>

#include <QFile>
#include <QDebug>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QShortcut>

#include <atomic>
#include <thread>
#include <chrono>

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

ApplicationManager::ApplicationManager(ApplicationQuitController* quitController_)
    : quitController(quitController_)
    , fileManager(new FileManager(this))
    , commandsSender(new AppsCommandsSender(this))
    , baManagerClient(new BAManagerClient(this))
    , configRefresher(new ConfigRefresher(this))
    , urlsHolder(new UrlsHolder(this))
    , mainWindow(new MainWindow(this))
    , taskManager(new TaskManager(this))
{
    using namespace std::placeholders;

    connect(mainWindow, &MainWindow::ShowPreferences, this, &ApplicationManager::OpenPreferencesEditor);

    connect(mainWindow, &MainWindow::CancelClicked, taskManager, &TaskManager::Terminate);

    //create secret shortcut
    //it will be used to get commands manually for testing reasons
    QShortcut* shortCut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_G), mainWindow);
    shortCut->setContext(Qt::ApplicationShortcut);
    connect(shortCut, &QShortcut::activated, baManagerClient, &BAManagerClient::AskForCommands);

    connect(mainWindow, &MainWindow::RefreshClicked, this, &ApplicationManager::Refresh);
    connect(configRefresher, &ConfigRefresher::RefreshConfig, this, &ApplicationManager::Refresh);

    localConfigFilePath = FileManager::GetDocumentsDirectory() + LOCAL_CONFIG_NAME;
    AddTaskWithBaseReceivers<LoadLocalConfigTask>(localConfigFilePath);

    ::LoadPreferences(fileManager, urlsHolder, baManagerClient, configRefresher);
}

ApplicationManager::~ApplicationManager()
{
    ::SavePreferences(fileManager, urlsHolder, baManagerClient, configRefresher);

    taskManager->Terminate();
    delete taskManager;
    delete mainWindow;
    delete urlsHolder;
}

void ApplicationManager::Start()
{
    mainWindow->show();
    Refresh();
}

void ApplicationManager::AddTaskWithBaseReceivers(std::unique_ptr<BaseTask>&& task)
{
    AddTaskWithCustomReceivers(std::move(task), { mainWindow->GetReceiver() });
}

void ApplicationManager::AddTaskWithCustomReceivers(std::unique_ptr<BaseTask>&& task, std::vector<Receiver> receivers)
{
    receivers.push_back(tasksLogger.GetReceiver());
    Notifier notifier(receivers);
    AddTaskWithNotifier(std::move(task), notifier);
}

void ApplicationManager::AddTaskWithNotifier(std::unique_ptr<BaseTask>&& task, const Notifier& notifier)
{
    taskManager->AddTask(std::move(task), notifier);
}

void ApplicationManager::InstallApplication(const InstallApplicationParams& params)
{
    AddTaskWithBaseReceivers<InstallApplicationTask>(params);
}

void ApplicationManager::Refresh()
{
    AddTaskWithBaseReceivers<UpdateConfigTask>(urlsHolder->GetURLs());
}

void ApplicationManager::OpenPreferencesEditor()
{
    PreferencesDialog::ShowPreferencesDialog(fileManager, urlsHolder, configRefresher, mainWindow);
}

void ApplicationManager::CheckUpdates()
{
    //TODO: remove it on version 30
    //this code used only to support version 27 and older
    FileManager::DeleteDirectory(fileManager->GetLauncherDirectory() + "temp/");

    //check self-update
    if (remoteConfig.GetLauncherVersion() != localConfig.GetLauncherVersion())
    {
        AddTaskWithBaseReceivers<SelfUpdateTask>(quitController, remoteConfig.GetLauncherURL());
        return;
    }

    //check applications update
    int branchCount = remoteConfig.GetBranchCount();
    for (int i = 0; i < branchCount; ++i)
    {
        bool toolsetWasFound = false;
        Branch* remoteBranch = remoteConfig.GetBranch(i);
        if (!localConfig.GetBranch(remoteBranch->id))
            continue;

        int appCount = remoteBranch->GetAppCount();
        for (int j = 0; j < appCount; ++j)
        {
            Application* remoteApp = remoteBranch->GetApplication(j);
            if (!localConfig.GetApplication(remoteBranch->id, remoteApp->id))
                continue;

            if (remoteApp->GetVerionsCount() == 1)
            {
                AppVersion* remoteAppVersion = remoteApp->GetVersion(0);
                Application* localApp = localConfig.GetApplication(remoteBranch->id, remoteApp->id);
                AppVersion* localAppVersion = localApp->GetVersion(0);
                if (localAppVersion->id != remoteAppVersion->id)
                {
                    if (remoteAppVersion->isToolSet)
                    {
                        if (toolsetWasFound)
                        {
                            continue;
                        }
                        toolsetWasFound = true;
                    }
                    InstallApplicationParams params;
                    params.branch = remoteBranch->id;
                    params.app = remoteApp->id;
                    params.newVersion = *remoteAppVersion;
                    InstallApplication(params);
                }
            }
        }
    }
}

const AppVersion* ApplicationManager::GetInstalledVersion(const QString& branchID, const QString& appID) const
{
    const Application* app = localConfig.GetApplication(branchID, appID);
    if (app == nullptr || app->versions.isEmpty())
    {
        return nullptr;
    }
    const AppVersion* version = app->GetVersion(0);
    return version;
}

AppVersion* ApplicationManager::GetInstalledVersion(const QString& branchID, const QString& appID)
{
    Application* app = localConfig.GetApplication(branchID, appID);
    if (app == nullptr || app->versions.isEmpty())
    {
        return nullptr;
    }
    AppVersion* version = app->GetVersion(0);
    return version;
}

void ApplicationManager::SaveLocalConfig() const
{
    localConfig.SaveToFile(localConfigFilePath);
}

bool ApplicationManager::CanTryStopApplication(const QString& applicationName) const
{
    return applicationName.contains("assetcacheserver", Qt::CaseInsensitive);
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

void ApplicationManager::OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version)
{
    localConfig.InsertApplication(branchID, appID, version);
    localConfig.UpdateApplicationsNames();
    SaveLocalConfig();
    if (mainWindow->GetSelectedBranchID() == branchID)
    {
        mainWindow->ShowTable(branchID);
    }
}

QString ApplicationManager::GetString(const QString& stringID) const
{
    QString string = remoteConfig.GetString(stringID);
    if (string == stringID)
        string = localConfig.GetString(stringID);
    return string;
}

ConfigParser* ApplicationManager::GetRemoteConfig()
{
    return &remoteConfig;
}

AppsCommandsSender* ApplicationManager::GetAppsCommandsSender() const
{
    return commandsSender;
}

MainWindow* ApplicationManager::GetMainWindow() const
{
    return mainWindow;
}

ConfigParser* ApplicationManager::GetLocalConfig()
{
    return &localConfig;
}

void ApplicationManager::ShowApplicataionInExplorer(const QString& branchID, const QString& appID)
{
    AppVersion* installedVersion = GetInstalledVersion(branchID, appID);
    if (installedVersion == nullptr)
    {
        return;
    }
    QString appDirPath = GetApplicationDirectory(branchID, appID, installedVersion->isToolSet, false);
    if (appDirPath.isEmpty())
    {
        return;
    }
    QtHelpers::ShowInOSFileManager(appDirPath);
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
    AddTaskWithBaseReceivers<RunApplicationTask>(branchID, appID, versionID);
}

void ApplicationManager::RemoveApplication(const QString& branchID, const QString& appID)
{
    AddTaskWithBaseReceivers<RemoveApplicationTask>(branchID, appID);
}

QString ApplicationManager::GetAppName(const QString& appName, bool isToolSet) const
{
    return isToolSet ? "Toolset" : appName;
}
