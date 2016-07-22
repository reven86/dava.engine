#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/CommandAction.h"

RECommandStack::RECommandStack()
    : DAVA::CommandStack()
{
    canRedoChanged.Connect(this, &RECommandStack::CanRedoChanged);
    canUndoChanged.Connect(this, &RECommandStack::CanUndoChanged);
    cleanChanged.Connect(this, &RECommandStack::EmitCleanChanged);
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
        DAVA::Command* commandPtr = commands.at(index).get();
        if (DAVA::IsCommandBatch(commandPtr))
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(static_cast<DAVA::CommandBatch*>(commandPtr));
            batch->RemoveCommands(commandId);
            if (batch->IsEmpty())
            {
                RemoveCommand(index);
            }
        }
        else if (IsRECommand(commandPtr))
        {
            const RECommand* reCommand = static_cast<const RECommand*>(commandPtr);
            if (reCommand->GetID() == commandId)
            {
                RemoveCommand(index);
            }
        }
    }
}

void RECommandStack::Activate()
{
    canUndoChanged.Emit(CanUndo());
    canRedoChanged.Emit(CanRedo());
}

bool RECommandStack::IsUncleanCommandExists(DAVA::uint32 commandId) const
{
    int beginIndex = std::max(cleanIndex, 0);
    for (DAVA::uint32 index = std::max(cleanIndex, 0), size = commands.size(); index < size; ++index)
    {
        const DAVA::Command* commandPtr = commands.at(index).get();
        if (IsRECommand(commandPtr))
        {
            const RECommand* reCommandPtr = static_cast<const RECommand*>(commandPtr);
            if (reCommandPtr->MatchCommandID(commandId))
            {
                return true;
            }
        }
    }
    return false;
}

void RECommandStack::RemoveCommand(DAVA::uint32 index)
{
    DVASSERT(index < commands.size());
    if (cleanIndex > static_cast<DAVA::int32>(index))
    {
        cleanIndex--;
    }
    commands.erase(commands.begin() + index);
    if (currentIndex > static_cast<DAVA::int32>(index))
    {
        SetCurrentIndex(currentIndex - 1);
    }
}
