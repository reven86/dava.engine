#include "Commands2/Base/CommandAction.h"

CommandAction::CommandAction(DAVA::int32 _id, const DAVA::String& _text)
    : Command2(_id, _text)
{
}

void CommandAction::Undo()
{
}

DAVA::Entity* CommandAction::GetEntity() const
{
    return nullptr;
}