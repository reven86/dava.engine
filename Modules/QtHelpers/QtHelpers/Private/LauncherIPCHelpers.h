#pragma once

#include <QCryptographicHash>
#include <QString>
//this is internal messages and replies
//user will never ever uses them on the client side

//this enums describe protocol-level of the OSI model

namespace LauncherIPCHelpers
{
enum eProtocolMessage
{
    PING = 0xF001
};
enum eProtocolReply
{
    PONG = 0xF007,
    WRONG_MESSAGE_FORMAT = 0xF00D,
    USER_REPLY = 0xffff
};

QString PathToKey(const QString &path);

}

inline QString LauncherIPCHelpers::PathToKey(const QString &path)
{
    return QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
}
