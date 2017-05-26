#pragma once

#include "Data/ConfigParser.h"
#include "Core/Receiver.h"
#include "Core/ApplicationQuitController.h"
#include "Core/Tasks/InstallApplicationTask.h"
#include "Core/TaskManager.h"
#include "Core/TasksLogger.h"
#include "Gui/MainWindow.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

#include <memory>

struct AppVersion;
class FileManager;
class AppsCommandsSender;
class BAManagerClient;
class ConfigRefresher;
class FileDownloader;
class UrlsHolder;

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(ApplicationQuitController* quitController);
    ~ApplicationManager();

    void Start();

    template <typename T, typename... Arguments>
    std::unique_ptr<BaseTask> CreateTask(Arguments&&... args);

    void AddTaskWithBaseReceivers(std::unique_ptr<BaseTask>&& task);
    void AddTaskWithCustomReceivers(std::unique_ptr<BaseTask>&& task, std::vector<Receiver> receivers);
    void AddTaskWithNotifier(std::unique_ptr<BaseTask>&& task, const Notifier& norifier);

    template <typename T, typename... Arguments>
    void AddTaskWithBaseReceivers(Arguments&&... args);

    void InstallApplication(const InstallApplicationParams& params);

    QString GetString(const QString& stringID) const;

    ConfigParser* GetLocalConfig();
    ConfigParser* GetRemoteConfig();
    const AppVersion* GetInstalledVersion(const QString& branchID, const QString& appID) const;
    AppVersion* GetInstalledVersion(const QString& branchID, const QString& appID);

    AppsCommandsSender* GetAppsCommandsSender() const;
    MainWindow* GetMainWindow() const;
    FileManager* GetFileManager() const;

    void OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version);

    void ShowApplicataionInExplorer(const QString& branchID, const QString& appID);
    void RunApplication(const QString& branchID, const QString& appID);
    void RunApplication(const QString& branchID, const QString& appID, const QString& versionID);

    void RemoveApplication(const QString& branchID, const QString& appID);

    void CheckUpdates();

    QString GetAppName(const QString& appName, bool isToolSet) const;

    void SaveLocalConfig() const;

    bool CanTryStopApplication(const QString& applicationName) const;

    QString GetApplicationDirectory(QString branchID, QString appID, bool isToolSet, bool mustExist = true) const;

    //this is a helper to get executable file name
    static QString GetLocalAppPath(const AppVersion* version, const QString& appID);

private slots:
    void Refresh();
    void OpenPreferencesEditor();

private:
    QString GetApplicationDirectory_kostil(const QString& branchID, const QString& appID) const;

    Notifier notifier;

    QString localConfigFilePath;

    ConfigParser localConfig;
    ConfigParser remoteConfig;

    TasksLogger tasksLogger;

    ApplicationQuitController* quitController = nullptr;

    FileManager* fileManager = nullptr;
    AppsCommandsSender* commandsSender = nullptr;

    BAManagerClient* baManagerClient = nullptr;
    ConfigRefresher* configRefresher = nullptr;

    UrlsHolder* urlsHolder = nullptr;

    MainWindow* mainWindow = nullptr;

    TaskManager* taskManager = nullptr;
};

template <typename T, typename... Arguments>
std::unique_ptr<BaseTask> ApplicationManager::CreateTask(Arguments&&... args)
{
    return std::make_unique<T>(this, std::forward<Arguments>(args)...);
}

template <typename T, typename... Arguments>
void ApplicationManager::AddTaskWithBaseReceivers(Arguments&&... args)
{
    AddTaskWithBaseReceivers(std::move(CreateTask<T>(std::forward<Arguments>(args)...)));
}
