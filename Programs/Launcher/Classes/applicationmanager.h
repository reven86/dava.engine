#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include "configparser.h"
#include "defines.h"
#include "updatedialog.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

class FileManager;
struct AppVersion;
class AppsCommandsSender;

class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(QObject* parent = 0);

    QString GetString(const QString& stringID) const;

    ConfigParser* GetLocalConfig();
    ConfigParser* GetRemoteConfig();
    const AppVersion* GetInstalledVersion(const QString& branchID, const QString& appID) const;
    AppVersion* GetInstalledVersion(const QString& branchID, const QString& appID);

    AppsCommandsSender* GetAppsCommandsSender() const;

    void OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version);

    void CheckUpdates(QQueue<UpdateTask>& tasks);

    void ShowApplicataionInExplorer(const QString& branchID, const QString& appID);
    void RunApplication(const QString& branchID, const QString& appID);
    void RunApplication(const QString& branchID, const QString& appID, const QString& versionID);
    //TODO:  silent need to be removed, all dialogs must be displayed on the client side
    bool RemoveApplication(const QString& branchID, const QString& appID, bool canReject, bool silent);
    bool RemoveBranch(const QString& branchID);
    bool PrepareToInstallNewApplication(const QString& branchID, const QString& appID, bool willInstallToolset, bool silent, QStringList& appsToRestart);

    bool ShouldShowNews();
    void NewsShowed();
    void ParseRemoteConfigData(const QByteArray& data);
    void SaveLocalConfig() const;

    QString GetApplicationDirectory(QString branchID, QString appID, bool isToolSet, bool mustExist = true) const;
    FileManager* GetFileManager() const;

    //this is a helper to get executable file name
    static QString GetLocalAppPath(const AppVersion* version, const QString& appID);

signals:
    void BranchChanged(const QString& branch);

private:
    void LoadLocalConfig(const QString& configPath);
    bool TryStopApp(const QString& runPath) const;
    bool CanRemoveApp(const QString& branchID, const QString& appID, bool canReject, bool silent) const;

    //before call this function check that app is not running
    bool RemoveApplicationImpl(const QString& branchID, const QString& appID, bool silent);
    bool CanTryStopApplication(const QString& applicationName) const;

    QString ExtractApplicationRunPath(const QString& branchID, const QString& appID, const QString& versionID);

    QString GetApplicationDirectory_kostil(const QString& branchID, const QString& appID) const;

    QString localConfigFilePath;

    ConfigParser localConfig;
    ConfigParser remoteConfig;

    FileManager* fileManager = nullptr;
    AppsCommandsSender* commandsSender = nullptr;
};

#endif // APPLICATIONMANAGER_H
