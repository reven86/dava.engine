#include "Command/Command.h"

namespace DAVA
{
Command::Command(int32 commandID, const String& description_)
    : id(commandID)
    , description(description_)
{
}
}
