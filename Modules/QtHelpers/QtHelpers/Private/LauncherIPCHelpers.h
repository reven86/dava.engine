#pragma once

#include <QCryptographicHash>
#include <QString>
//this is internal messages and replies
//user will never ever uses them on the client side

//this enums describe protocol-level of the OSI model

namespace LauncherIPCHelpers
{
//enum initial values must be greater then data layer values
enum eProtocolMessage
{
    PING = 0xf000
};
enum eProtocolReply
{
    PONG = 0xf000,
    USER_REPLY = 0xffff
};

QString PathToKey(const QString& path);
}

inline QString LauncherIPCHelpers::PathToKey(const QString& path)
{
    return QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex();
}
