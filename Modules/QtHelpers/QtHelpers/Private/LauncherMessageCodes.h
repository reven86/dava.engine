#pragma once

//this is internal messages and replies
//user will never ever uses them on the client side
namespace LauncherMessageCodes
{
enum class eMessageInternal
{
    PING = 0xF001
};
enum class eReplyInternal
{
    PONG = 0xF007,
    WRONG_MESSAGE_FORMAT = 0xF00D
};
}
