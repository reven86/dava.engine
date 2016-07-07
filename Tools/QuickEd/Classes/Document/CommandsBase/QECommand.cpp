#include "Document/CommandsBase/QECommand.h"

QECommand::QECommand(DAVA::CommandID_t id, const DAVA::String& text)
    : Command(id, text)
{
}

void QECommand::Execute()
{
    Redo();
}
