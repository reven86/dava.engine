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
    QApplication* application = qApp;
    if (application == nullptr)
    {
        lastError = "LauncherListener: application was not initialized";
        return false;
    }
#ifdef Q_OS_MAC
    CFURLRef url = (CFURLRef)CFAutorelease((CFURLRef)CFBundleCopyBundleURL(CFBundleGetMainBundle()));
    QString appPath = QUrl::fromCFURL(url).path();
    //launcher will use app as a file, but not as a folder
    while (appPath.endsWith('/'))
    {
        appPath.chop(1);
    }
    //sometimes CFBundleCopyBundleURL returns url with a double slashes
    appPath.replace("//", "/");
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
    QLocalSocket* clientConnection = server->nextPendingConnection();

    QObject::connect(clientConnection, &QLocalSocket::disconnected, clientConnection, &QLocalSocket::deleteLater);
    QObject::connect(clientConnection, &QLocalSocket::readyRead, this, &LauncherListener::ProcessTransportLevel);
}

void LauncherListener::ProcessTransportLevel()
{
    QLocalSocket* clientConnection = qobject_cast<QLocalSocket*>(sender());
    if (clientConnection == nullptr)
    {
        qCritical() << "internal error, OnReadyRead called not from QLocalSocket";
        return;
    }
    QByteArray data = clientConnection->readAll();
    //if this a channel-level message, like ping or handshake, process it and return replt to the sender
    int replyCode = ProcessChannelLevel(data);
    //else process message on the data level
    if (replyCode == UNKNOWN_MESSAGE)
    {
        replyCode = ProcessDataLevel(data);
    }
    QByteArray reply = QByteArray::number(static_cast<int>(replyCode));
    clientConnection->write(reply);
    clientConnection->flush();
    clientConnection->disconnectFromServer();
}

int LauncherListener::ProcessChannelLevel(const QByteArray& data)
{
    using namespace LauncherIPCHelpers;
    bool ok = false;
    int code = data.toInt(&ok);
    if (ok)
    {
        eProtocolMessage message = static_cast<eProtocolMessage>(code);
        if (message == PING)
        {
            return LauncherIPCHelpers::PONG;
        }
    }
    return UNKNOWN_MESSAGE;
}

int LauncherListener::ProcessDataLevel(const QByteArray& data)
{
    bool ok = false;
    int messageCode = data.toInt(&ok);
    if (ok)
    {
        eMessage message = static_cast<eMessage>(messageCode);
        return processRequest(message);
    }
    return UNKNOWN_MESSAGE;
}
