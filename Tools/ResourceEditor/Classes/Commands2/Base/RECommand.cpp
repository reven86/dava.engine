#include "Commands2/Base/RECommand.h"

RECommand::RECommand(DAVA::CommandID_t id, const DAVA::String& text)
    : Command(id, text)
{
}

RECommand::~RECommand() = default;

DAVA::Command::Pointer RECommand::CreateEmptyCommand()
{
    return Pointer();
}

DAVA::Entity* RECommand::GetEntity() const
{
    return nullptr;
}

void RECommand::Execute()
{
    Redo();
}
