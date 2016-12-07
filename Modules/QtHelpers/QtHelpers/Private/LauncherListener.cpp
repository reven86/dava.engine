#include "QtHelpers/LauncherListener.h"
#include "QtHelpers/Private/LauncherIPCHelpers.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#ifdef Q_OS_MAC
#include <QUrl>
#include <CoreFoundation/CoreFoundation.h>
#endif //Q_OS_MAC

LauncherListener::LauncherListener()
{
    server.reset(new QLocalServer());
}

LauncherListener::~LauncherListener()
{
    server->close();
}

bool LauncherListener::Init(ProcessRequestFunction function)
{
    QApplication *application = qApp;
    if (application == nullptr)
    {
        lastError = "LauncherListener: application was not initialized";
        return false;
    }
#ifdef Q_OS_MAC
    CFURLRef url = (CFURLRef)CFAutorelease((CFURLRef)CFBundleCopyBundleURL(CFBundleGetMainBundle()));
    QString appPath = QUrl::fromCFURL(url).path();
    while(appPath.endsWith('/'))
    {
        appPath.chop(1);
    }
#else
    QString appPath = application->applicationFilePath();
#endif //platform
    appPath = LauncherIPCHelpers::PathToKey(appPath);

    //if last appplication was crashed - server will hold in OS untill we'll remove it manually
    //works only on Unix systems
    server->removeServer(appPath);
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
    using namespace LauncherIPCHelpers;
    QLocalSocket *clientConnection = qobject_cast<QLocalSocket*>(sender());
    if (clientConnection == nullptr)
    {
        qCritical() << "internal error, OnReadyRead called not from QLocalSocket";
        return;
    }
    QByteArray data = clientConnection->readAll();
    bool ok = false;
    int code = data.toInt(&ok);
    eProtocolReply replyInternal = WRONG_MESSAGE_FORMAT;
    if (ok)
    {
        eMessage message = static_cast<eMessage>(code);
        if (static_cast<eProtocolMessage>(message) == PING)
        {
            replyInternal = LauncherIPCHelpers::PONG;
        }
        else
        {
            eReply reply = processRequest(message);
            replyInternal = static_cast<eProtocolReply>(reply);
        }
    }
    QByteArray reply = QByteArray::number(static_cast<int>(replyInternal));
    clientConnection->write(reply);
    clientConnection->flush();
    clientConnection->disconnectFromServer();
}
