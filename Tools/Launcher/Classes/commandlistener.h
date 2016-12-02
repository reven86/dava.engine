#pragma once

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QProcess>

class SilentUpdater;
class ApplicationManager;
class QTimer;
class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject;

class CommandListener : public QObject
{
    Q_OBJECT

public:
    enum eResult
    {
        //this values is BA-manager ret-code requirements
        SUCCESS = 2,
        FAILURE = 3
    };
    CommandListener(ApplicationManager* appManager, QObject* parent = nullptr);

    void AskForCommands();

    QString GetProtocolKey() const;
    void SetProtocolKey(const QString& key);

private slots:

    void OnReply(QNetworkReply* reply);
    void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void OnProcessError(QProcess::ProcessError error);

private:
    void ProcessCommand(const QJsonObject& object);
    void Post(const QString& urlStr, const QByteArray& data);
    void SendReply(eResult result, const QString& commandID, const QString& message);
    void SendReply(eResult result, const QString& message);

    void LaunchProcess(const QJsonObject& requestObj, const QString& commandIDValue);
    void SilentUpdate(const QJsonObject& requestObj, const QString& commandIDValue);

    SilentUpdater* silentUpdater = nullptr;
    ApplicationManager* applicationManager = nullptr;
    QTimer* updateTimer = nullptr;
    QNetworkAccessManager* networkManager = nullptr;

    QByteArray protocolKey;
    QMap<QProcess*, QString> startedCommandIDs;
};
