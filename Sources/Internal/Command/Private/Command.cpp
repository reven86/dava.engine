#include "Command/Command.h"

namespace DAVA
{
Command::Command(CommandID commandID, const String& description_)
    : id(commandID)
    , description(description_)
{
}
}
