#include "Command/Command.h"

namespace DAVA
{
Command::Command(CommandID_t id_, const String& text_)
    : id(id_)
    , text(text_)
{
}
}
