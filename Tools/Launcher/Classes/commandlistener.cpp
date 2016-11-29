#include "CommandListener.h"
#include "applicationmanager.h"
#include "errormessenger.h"
#include "silentupdater.h"

#include "defines.h"

#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#include <QProcess>
#include <QDir>

CommandListener::CommandListener(ApplicationManager* appManager_, QObject* parent /* = nullptr */)
    : QObject(parent)
    , silentUpdater(new SilentUpdater(appManager_, this))
    , applicationManager(appManager_)
    , updateTimer(new QTimer(this))
    , networkManager(new QNetworkAccessManager(this))
{
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(1 * 1000);
    connect(updateTimer, &QTimer::timeout, this, &CommandListener::GetCommands);
    connect(networkManager, &QNetworkAccessManager::finished, this, &CommandListener::GotReply);
    //updateTimer->start();
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
    else
    {
        SendReply(FAILURE, commandIDStr, QString("Unknown command: %1").arg(typeStr));
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
    qDebug() << "reply data: " << data;
    Post(urlStr, data);

    if (result != SUCCESS)
    {
        ErrorMessenger::LogMessage(QtWarningMsg, message);
    }
}

void CommandListener::SendReply(eResult result, const QString& message)
{
    SendReply(result, "0", message);
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
    Post(urlStr, data);
}

void CommandListener::Post(const QString& urlStr, const QByteArray& data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    networkManager->post(request, data);
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
    QByteArray readedData = reply->readAll();
    qDebug() << readedData;
    QJsonDocument document = QJsonDocument::fromJson(readedData, &parseError);
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
    else if (keyValue.isNull() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(keyValue.type()));
        SendReply(FAILURE, "Can not parse key from reply, expect a string, got type " + jsonValueTypeStr);
    }
    QJsonValue commandsValue = rootObj["commands"];
    if (commandsValue.isArray() == false && commandsValue.isNull() == false)
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
                SendReply(FAILURE, "Got a command, which not an object");
            }
        }
    }
    QJsonValue commandResultValue = rootObj["command_result"];
    if (commandResultValue.isArray() == false && commandResultValue.isNull() == false)
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
    process->deleteLater();

    QString commandID = startedCommandIDs[process];
    startedCommandIDs.remove(process);
    if (commandID.isEmpty())
    {
        ErrorMessenger::LogMessage(QtWarningMsg, "Internal error: process is not registered");
        return;
    }
    QByteArray standardOutput = process->readAllStandardOutput();
    QByteArray standardError = process->readAllStandardError();
    eResult result = (exitCode == 0 && exitStatus == QProcess::NormalExit) ? SUCCESS : FAILURE;
    QString reply = QString("process exit code: %1\nprocess exit status: %2\nstandard output: %3\nstandard error: %4");
    reply = reply.arg(exitCode).arg(static_cast<int>(exitStatus)).arg(QString(standardOutput)).arg(QString(standardError));
    SendReply(result, commandID, reply);
}

void CommandListener::OnProcessError(QProcess::ProcessError error)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (process == nullptr)
    {
        ErrorMessenger::LogMessage(QtWarningMsg, "Internal error: got Process Error not from process");
        return;
    }
    process->deleteLater();

    QString commandID = startedCommandIDs[process];
    if (commandID.isEmpty())
    {
        ErrorMessenger::LogMessage(QtWarningMsg, "Internal error: process is not registered");
        return;
    }
    QString reply = QString("Error occured while launching process. Qt error code is %1").arg(static_cast<int>(error));
    SendReply(FAILURE, commandID, reply);
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
    ErrorMessenger::LogMessage(QtDebugMsg, "launched command from BA-manager: " + cmd);
    QProcess* process = new QProcess();
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &CommandListener::OnProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &CommandListener::OnProcessError);
    startedCommandIDs[process] = commandID;
    process->start("cmd.exe /c " + cmd, QProcess::ReadWrite);
}

void CommandListener::SilentUpdate(const QJsonObject& requestObj, const QString& commandID)
{
    QJsonValue branchNameValue = requestObj["branchName"];
    if (branchNameValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(branchNameValue.type()));
        SendReply(FAILURE, commandID, "Can not parse branch name from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }
    QString branchName = branchNameValue.toString();

    QJsonValue appNameValue = requestObj["build_name"];
    if (appNameValue.isString() == false)
    {
        QString jsonValueTypeStr = QString::number(static_cast<int>(appNameValue.type()));
        SendReply(FAILURE, commandID, "Can not parse branch name from command, expect a string, got type " + jsonValueTypeStr);
        return;
    }
    QString appName = appNameValue.toString();
    appName = applicationManager->GetString(appName);
    AppVersion newVersion;
    ::FillAppFields(&newVersion, requestObj, IsToolset(appName));
    AppVersion* installedVersion = applicationManager->GetInstalledVersion(branchName, appName);
    SilentUpdateTask::CallBack callBack = [this, commandID](bool success, const QString& message) {
        SendReply(success ? SUCCESS : FAILURE, commandID, message);
    };
    SilentUpdateTask task(branchName, appName, installedVersion, newVersion, callBack);
    silentUpdater->AddTask(std::move(task));
}
