#include "QtTools/Commands/CommandWithoutExecute.h"

CommandWithoutExecute::CommandWithoutExecute(DAVA::CommandID_t id, const DAVA::String& text /* = "" */)
    : DAVA::Command(id, text)
{
}

void CommandWithoutExecute::Execute()
{
    Redo();
}
