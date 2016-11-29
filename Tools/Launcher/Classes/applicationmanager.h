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
class ProcessCommunication;

class ApplicationManager : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationManager(QObject* parent = 0);

    QString GetString(const QString& stringID) const;

    ConfigParser* GetLocalConfig();
    ConfigParser* GetRemoteConfig();
    AppVersion* GetInstalledVersion(const QString& branchID, const QString& appID);

    ProcessCommunication* GetProcessCommunicationModule() const;

    void OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version);

    void CheckUpdates(QQueue<UpdateTask>& tasks);

    void ShowApplicataionInExplorer(const QString& branchID, const QString& appID, const QString& versionID);
    void RunApplication(const QString& branchID, const QString& appID);
    void RunApplication(const QString& branchID, const QString& appID, const QString& versionID);
    bool RemoveApplication(const QString& branchID, const QString& appID, bool canReject);
    bool RemoveBranch(const QString& branchID);
    bool PrepareToInstallNewApplication(const QString& branchID, const QString& appID, bool willInstallToolset, QStringList& appsToRestart);

    bool ShouldShowNews();
    void NewsShowed();
    void ParseRemoteConfigData(const QByteArray& data);
    void SaveLocalConfig() const;

    QString GetApplicationDirectory(QString branchID, QString appID, bool isToolSet, bool mustExist = true) const;
    FileManager* GetFileManager() const;

    //this is a helper to get executable file name
    static QString GetLocalAppPath(const AppVersion* version, const QString& appID);

private:
    void LoadLocalConfig(const QString& configPath);
    bool TryStopApp(const QString& runPath) const;

    //before call this function check that app is not running
    bool RemoveApplicationImpl(const QString& branchID, const QString& appID);

    QString ExtractApplicationRunPath(const QString& branchID, const QString& appID, const QString& versionID);

    QString GetApplicationDirectory_kostil(const QString& branchID, const QString& appID) const;

    QString localConfigFilePath;

    ConfigParser localConfig;
    ConfigParser remoteConfig;

    FileManager* fileManager = nullptr;
    ProcessCommunication* processCommunication = nullptr;
};

#endif // APPLICATIONMANAGER_H
