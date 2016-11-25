#pragma once

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QProcess>

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
        SUCCESS,
        FAILURE
    };
    CommandListener(ApplicationManager* appManager, QObject* parent = nullptr);

    QString GetProtocolKey() const;
    void SetProtocolKey(const QString& key);
    void ProcessCommand(const QJsonObject& object);

    void SendReply(eResult result = SUCCESS, const QString& commandID = "0", const QString& message = "Success");

private slots:
    void GetCommands();
    void GotReply(QNetworkReply* reply);
    void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void LaunchProcess(const QJsonObject& requestObj, const QString& commandIDValue);
    void SilentUpdate(const QJsonObject& requestObj, const QString& commandIDValue);
    ApplicationManager* applicationManager = nullptr;
    QTimer* updateTimer = nullptr;
    QNetworkAccessManager* networkManager = nullptr;

    QByteArray protocolKey;
    QMap<QProcess*, QString> startedCommandIDs;
};
