#include "appscommandssender.h"
#include "QtHelpers/LauncherListener.h"
#include "QtHelpers/Private/LauncherMessageCodes.h"

#include <QEventLoop>
#include <QTimer>

namespace AppsCommandsSenderDetails
{
enum class eReplyInternal
{
    TARGET_NOT_FOUND = static_cast<int>(LauncherListener::eReply::REPLIES_COUNT),
    TIMEOUT_ERROR,
    OTHER_ERROR
};
}

AppsCommandsSender::AppsCommandsSender(QObject* parent)
    : QObject(parent)
    , socket(new QLocalSocket(this))
{
}

AppsCommandsSender::~AppsCommandsSender()
{
    if (socket->state() != QLocalSocket::UnconnectedState)
    {
        socket->abort();
    }
}

bool AppsCommandsSender::HostIsAvailable(const QString& appPath)
{
    Q_ASSERT(socket->state() == QLocalSocket::UnconnectedState);

    socket->connectToServer(appPath);
    socket->waitForConnected();
    if (socket->state() == QLocalSocket::ConnectedState)
    {
        socket->disconnectFromServer();
        return true;
    }
    return false;
}

bool AppsCommandsSender::Ping(const QString& appPath)
{
    int pingCode = static_cast<int>(LauncherMessageCodes::eMessageInternal::PING);
    int replyCode = SendMessage(pingCode, appPath);
    return static_cast<LauncherMessageCodes::eReplyInternal>(replyCode) == LauncherMessageCodes::eReplyInternal::PONG;
}

bool AppsCommandsSender::RequestQuit(const QString& appPath)
{
    using namespace AppsCommandsSenderDetails;
    int quitCode = static_cast<int>(LauncherListener::eMessage::QUIT);
    int replyCode = SendMessage(quitCode, appPath);
    return static_cast<eReplyInternal>(replyCode) == eReplyInternal::TARGET_NOT_FOUND
    || static_cast<LauncherListener::eReply>(replyCode) == LauncherListener::eReply::ACCEPT;
}

int AppsCommandsSender::SendMessage(int message, const QString& appPath)
{
    using namespace AppsCommandsSenderDetails;

    Q_ASSERT(socket->state() == QLocalSocket::UnconnectedState);
    socket->connectToServer(appPath);
    socket->waitForConnected();
    if (socket->state() == QLocalSocket::ConnectedState)
    {
        QByteArray data = socket->readAll();

        QEventLoop eventLoop;
        QTimer waitTimer(this);
        waitTimer.setSingleShot(true);
        connect(socket, &QLocalSocket::readyRead, &eventLoop, &QEventLoop::quit);
        connect(&waitTimer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);

        waitTimer.start(10 * 1000); //wait 10 seconds for reply
        socket->write(QByteArray::number(message));
        eventLoop.exec();
        data = socket->readAll();
        socket->disconnectFromServer();
        //this situation occurs when application fails on processing message
        if (waitTimer.isActive() == false)
        {
            return static_cast<int>(eReplyInternal::TIMEOUT_ERROR);
        }
        bool ok = false;
        int code = data.toInt(&ok);
        if (ok)
        {
            return code;
        }
    }
    else
    {
        QLocalSocket::LocalSocketError lastError = socket->error();
        if (lastError == QLocalSocket::ServerNotFoundError)
        {
            return static_cast<int>(eReplyInternal::TARGET_NOT_FOUND);
        }
    }
    return static_cast<int>(eReplyInternal::OTHER_ERROR);
}
