#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/CommandAction.h"

RECommandStack::RECommandStack()
    : DAVA::CommandStack()
{
    canRedoChanged.Connect(DAVA::MakeFunction(this, &RECommandStack::EmitUndoRedoStateChanged));
    canUndoChanged.Connect(DAVA::MakeFunction(this, &RECommandStack::EmitUndoRedoStateChanged));
    cleanChanged.Connect(DAVA::MakeFunction(this, &RECommandStack::EmitCleanChanged));
}

void RECommandStack::Clear()
{
    commands.clear();
    cleanIndex = EMPTY_INDEX;
    SetCurrentIndex(EMPTY_INDEX);
}

void RECommandStack::RemoveCommands(DAVA::uint32 commandId)
{
    for (DAVA::uint32 index = 0; index < commands.size(); ++index)
    {
        const DAVA::Command::Pointer& command = *iter;
        if (command->GetID() == DAVA::CMDID_BATCH)
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(cmd);
            batch->RemoveCommands(commandId);
            if (batch->IsEmpty())
            {
                RemoveCommand(index);
            }
        }
        else if (command->GetID() == commandId)
        {
            RemoveCommand(index);
        }
    }
}

void RECommandStack::Activate()
{
    EmitUndoRedoStateChanged();
}

bool RECommandStack::IsUncleanCommandExists(DAVA::uint32 commandId) const
{
    int beginIndex = std::max(cleanIndex, 0);
    for (DAVA::uint32 index = std::max(cleanIndex, 0), size = commands.size(); index < size; ++index)
    {
        if (commands->at(index)->MatchCommandID(commandId))
        {
            return true;
        }
    }
    return false;
}

void RECommandStack::RemoveCommand(DAVA::uint32 index)
{
    DVASSERT(index < commands.size());
    if (cleanIndex > index)
    {
        cleanIndex--;
    }
    commands.erase(commands.begin() + index);
    if (currentIndex > index)
    {
        SetCurrentIndex(currentIndex - 1);
    }
}
