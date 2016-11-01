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
class ApplicationManager : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationManager(QObject* parent = 0);

    QString GetString(const QString& stringID) const;

    ConfigParser* GetLocalConfig();
    ConfigParser* GetRemoteConfig();

    void CheckUpdates(QQueue<UpdateTask>& tasks);

    void ShowApplicataionInExplorer(const QString& branchID, const QString& appID, const QString& versionID);
    void RunApplication(const QString& branchID, const QString& appID, const QString& versionID);
    bool RemoveApplication(const QString& branchID, const QString& appID, const QString& versionID);
    bool RemoveBranch(const QString& branchID);

    bool ShouldShowNews();
    void NewsShowed();
    void ParseRemoteConfigData(const QByteArray& data);
    void SaveLocalConfig() const;

    QString GetApplicationDirectory(QString branchID, QString appID, bool mustExist = true) const;
    FileManager* GetFileManager() const;

public slots:
    void OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version);

private:
    void LoadLocalConfig(const QString& configPath);
    QString ExtractApplicationRunPath(const QString& branchID, const QString& appID, const QString& versionID);

    QString GetApplicationDirectory_kostil(const QString& branchID, const QString& appID) const;

    QString localConfigFilePath;

    ConfigParser localConfig;
    ConfigParser remoteConfig;

    FileManager* fileManager = nullptr;
};

#endif // APPLICATIONMANAGER_H
