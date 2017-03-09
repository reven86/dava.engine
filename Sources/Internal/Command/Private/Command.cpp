#include "Command/Command.h"

namespace DAVA
{
Command::Command(const String& description_)
    : description(description_)
{
}

Command::Command(CommandID commandID, const String& description_)
    : id(commandID)
    , description(description_)
{
}
}
