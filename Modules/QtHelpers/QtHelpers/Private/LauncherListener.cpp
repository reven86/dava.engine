#include "QtHelpers/LauncherListener.h"
#include "QtHelpers/Private/LauncherMessageCodes.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QApplication>
#include <QTimer>
#include <QDebug>

LauncherListener::LauncherListener() = default;
LauncherListener::~LauncherListener() = default;

bool LauncherListener::Init(ProcessRequestFunction function)
{
    server.reset(new QLocalServer());
    QApplication *application = qApp;
    if (application == nullptr)
    {
        lastError = "LauncherListener: application was not initialized";
        return false;
    }
    QString appPath = application->applicationFilePath();
    if (server->listen(appPath) == false)
    {
        lastError = "unable to listen server: " + server->errorString();
        return false;
    }

    QObject::connect(server.get(), &QLocalServer::newConnection, std::bind(&LauncherListener::OnNewConnection, this));
    processRequest = function;

    return true;
}

void LauncherListener::OnNewConnection()
{
    QLocalSocket *clientConnection = server->nextPendingConnection();

    QObject::connect(clientConnection, &QLocalSocket::disconnected, clientConnection, &QLocalSocket::deleteLater);
    QObject::connect(clientConnection, &QLocalSocket::readyRead, this, &LauncherListener::OnReadyRead);
}

void LauncherListener::OnReadyRead()
{
    QLocalSocket *clientConnection = qobject_cast<QLocalSocket*>(sender());
    if (clientConnection == nullptr)
    {
        qCritical() << "internal error, OnReadyRead called not from QLocalSocket";
        return;
    }
    QByteArray data = clientConnection->readAll();
    bool ok = false;
    int code = data.toInt(&ok);
    LauncherMessageCodes::eReplyInternal replyInternal = LauncherMessageCodes::eReplyInternal::WRONG_MESSAGE_FORMAT;
    if (ok)
    {
        eMessage message = static_cast<eMessage>(code);
        if (static_cast<LauncherMessageCodes::eMessageInternal>(message) == LauncherMessageCodes::eMessageInternal::PING)
        {
            replyInternal = LauncherMessageCodes::eReplyInternal::PONG;
        }
        else
        {
            eReply reply = processRequest(message);
            replyInternal = static_cast<LauncherMessageCodes::eReplyInternal>(reply);
        }
    }
    QByteArray reply = QByteArray::number(static_cast<int>(replyInternal));
    clientConnection->write(reply);
    clientConnection->flush();
    clientConnection->disconnectFromServer();
}
