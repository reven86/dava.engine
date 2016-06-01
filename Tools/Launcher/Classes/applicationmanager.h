#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include "configparser.h"
#include "defines.h"
#include "updatedialog.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

class ConfigDownloader;
class ApplicationManager : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationManager(QObject* parent = 0);

    QString GetString(const QString& stringID) const;

    ConfigParser* GetLocalConfig();
    ConfigParser* GetRemoteConfig();

    void CheckUpdates(QQueue<UpdateTask>& tasks);

    void RunApplication(const QString& branchID, const QString& appID, const QString& versionID);
    bool RemoveApplication(const QString& branchID, const QString& appID, const QString& versionID);
    bool RemoveBranch(const QString& branchID);

    bool ShouldShowNews();
    void NewsShowed();

signals:
    void Refresh();

public slots:
    void OnAppInstalled(const QString& branchID, const QString& appID, const AppVersion& version);

private:
    void LoadLocalConfig(const QString& configPath);
    void ParseRemoteConfigData(const QByteArray& data);

    QString localConfigFilePath;

    ConfigParser localConfig;
    ConfigParser remoteConfig;

    friend class ConfigDownloader;
};

#endif // APPLICATIONMANAGER_H
