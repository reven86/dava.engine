#include "CommandListener.h"
#include "applicationmanager.h"
#include "errormessenger.h"

#include "defines.h"

#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#include <QProcess>

CommandListener::CommandListener(ApplicationManager* appManager_, QObject* parent /* = nullptr */)
    : QObject(parent)
    , applicationManager(appManager_)
    , updateTimer(new QTimer(this))
    , networkManager(new QNetworkAccessManager(this))
{
    updateTimer->setSingleShot(false);
    updateTimer->setInterval(90 * 1000);
    connect(updateTimer, &QTimer::timeout, this, &CommandListener::GetCommands);
    connect(networkManager, &QNetworkAccessManager::finished, this, &CommandListener::GotReply);
}

QString CommandListener::GetProtocolKey() const
{
    return protocolKey;
}

void CommandListener::SetProtocolKey(const QString& key)
{
    protocolKey = key.toUtf8();
}

void CommandListener::ProcessCommand(const QJsonObject& object)
{
    QJsonValue commandIDValue = object["command_id"];
    if (commandIDValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(commandIDValue.type()));
        SendReply(FAILURE, "Can not parse command ID from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }
    QString commandIDStr = commandIDValue.toString();

    QJsonValue typeValue = object["type"];
    if (typeValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(typeValue.type()));
        SendReply(FAILURE, commandIDStr, "Can not parse type from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }

    QString typeStr = typeValue.toString();
    if (typeStr == "command")
    {
        LaunchProcess(object, commandIDStr);
    }
    else if (typeStr == "install")
    {
        SilentUpdate(object, commandIDStr);
    }
}

void CommandListener::SendReply(eResult result, const QString& commandID, const QString& message)
{
    QString urlStr = "http://ba-manager.wargaming.net/panel/modules/jsonAPI/launcher/lite.php";
    QByteArray data = "source=command_result";
    data.append("&command_id=" + commandID);
    data.append("&key=");
    QString key = GetProtocolKey();
    data.append(key.isEmpty() ? "NULL" : key);
    data.append("&res_code=");
    data.append(QString::number(static_cast<int>(result)));
    data.append("&message='");
    data.append(message);
    data.append("'");
    networkManager->post(QNetworkRequest(QUrl(urlStr)), data);

    if (result != SUCCESS)
    {
        ErrorMessenger::LogMessage(QtWarningMsg, message);
    }
}

void CommandListener::GetCommands()
{
    QString urlStr = "http://ba-manager.wargaming.net/panel/modules/jsonAPI/launcher/lite.php";
    QByteArray data = "source=commands";
    data.append("&key=");
    QString key = GetProtocolKey();
    data.append(key.isEmpty() ? "NULL" : key);
    data.append("&os=");
    data.append(platformString);
    networkManager->post(QNetworkRequest(QUrl(urlStr)), data);
}

void CommandListener::GotReply(QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError)
    {
        ErrorMessenger::LogMessage(QtWarningMsg, "Network error: " + reply->errorString());
        return;
    }

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        SendReply(FAILURE, "Can not parse JSON reply, error is: " + parseError.errorString());
        return;
    }
    QJsonObject rootObj = document.object();
    QJsonValue keyValue = rootObj["key"];
    if (keyValue.isString())
    {
        SetProtocolKey(keyValue.toString());
    }
    else if (keyValue.isUndefined() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(keyValue.type()));
        SendReply(FAILURE, "Can not parse key from reply, expect a string, got type " + jsonValueTypeStr);
    }
    QJsonValue commandsValue = rootObj["commands"];
    if (commandsValue.isArray() == false && commandsValue.isUndefined() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(commandsValue.type()));
        SendReply(FAILURE, "Can not parse commands from reply, expects an array, got type " + jsonValueTypeStr);
    }
    else
    {
        QJsonArray commands = commandsValue.toArray();
        for (const QJsonValueRef& valueRef : commands)
        {
            if (valueRef.isObject())
            {
                ProcessCommand(valueRef.toObject());
            }
            else
            {
                SendReply(FAILURE, "Got a command, which is not object");
            }
        }
    }
    QJsonValue commandResultValue = rootObj["command_result"];
    if (commandsValue.isArray() == false && commandsValue.isUndefined() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(commandResultValue.type()));
        SendReply(FAILURE, "Can not parse command result from reply, expects an array, got type " + jsonValueTypeStr);
    }
    else
    {
        //we have not any requirements to process answer result.
    }
}

void CommandListener::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (process == nullptr)
    {
        ErrorMessenger::LogMessage(QtWarningMsg, "Internal error: got Process Finished not from process");
        return;
    }
    QString commandID = startedCommandIDs[process];
    if (commandID.isEmpty())
    {
        ErrorMessenger::LogMessage(QtWarningMsg, "Internal error: process is not registered");
    }
    QByteArray standardOutput = process->readAllStandardOutput();
    QByteArray standardError = process->readAllStandardError();
    eResult result = (exitCode == 0 && exitStatus == QProcess::NormalExit) ? SUCCESS : FAILURE;
    QString reply = QString("process exit code: %1\nprocess exit status: %2\nstandard output: %3\nstandard error: %4");
    reply = reply.arg(exitCode).arg(static_cast<int>(exitStatus)).arg(QString(standardOutput)).arg(QString(standardError));
    SendReply(result, commandID, reply);
    process->deleteLater();
}

void CommandListener::LaunchProcess(const QJsonObject& requestObj, const QString& commandID)
{
    QJsonValue commandValue = requestObj["cmd"];
    if (commandValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(commandValue.type()));
        SendReply(FAILURE, commandID, "Can not parse cmd from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }

    QString cmd = commandValue.toString();
    QProcess* process = new QProcess();
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &CommandListener::OnProcessFinished);
    startedCommandIDs[process] = commandID;
    process->start(cmd, QProcess::ReadWrite);
}

void CommandListener::SilentUpdate(const QJsonObject& requestObj, const QString& commandID)
{
    QJsonValue buildNameValue = requestObj["build_name"];
    if (buildNameValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(buildNameValue.type()));
        SendReply(FAILURE, commandID, "Can not parse build name from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }
    QString buildName = buildNameValue.toString();

    QJsonValue branchNameValue = requestObj["branchName"];
    if (branchNameValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(branchNameValue.type()));
        SendReply(FAILURE, commandID, "Can not parse branch name from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }
    QString branchName = branchNameValue.toString();
}
