#include "Commands2/Base/CommandStack.h"
#include "Commands2/Base/CommandAction.h"

CommandStack::CommandStack()
{
}

CommandStack::~CommandStack()
{
    Clear();
}

bool CommandStack::CanUndo() const
{
    return (!commandList.empty() && nextCommandIndex > 0);
}

bool CommandStack::CanRedo() const
{
    return (nextCommandIndex < static_cast<DAVA::int32>(commandList.size()));
}

void CommandStack::Clear()
{
    nextCommandIndex = 0;
    nextAfterCleanCommandIndex = 0;
    commandList.clear();
    CleanCheck();
}

void CommandStack::RemoveCommands(DAVA::int32 commandId)
{
    DAVA::int32 index = 0;
    for (auto it = commandList.begin(), endIt = commandList.end(); it != endIt;)
    {
        Command2* command = (*it).get();
        DVASSERT(command != nullptr);

        bool shouldRemoveCommand = (command->GetId() == commandId);
        if (!shouldRemoveCommand && (command->GetId() == CMDID_BATCH))
        {
            CommandBatch* batch = static_cast<CommandBatch*>(command);
            batch->RemoveCommands(commandId);
            shouldRemoveCommand = batch->Empty();
        }

        if (shouldRemoveCommand)
        {
            it = commandList.erase(it);

            nextCommandIndex = DAVA::Max(nextCommandIndex - 1, 0);

            if (index < nextAfterCleanCommandIndex)
            {
                nextAfterCleanCommandIndex = DAVA::Max(nextAfterCleanCommandIndex - 1, 0);
            }
        }
        else
        {
            ++index;
            ++it;
        }
    }

    CleanCheck();
}

void CommandStack::Undo()
{
    if (CanUndo())
    {
        nextCommandIndex--;
        Command2* commandToUndo = GetCommandInternal(nextCommandIndex);

        if (nullptr != commandToUndo)
        {
            commandToUndo->Undo();
            EmitNotify(commandToUndo, false);
        }
    }

    CleanCheck();
}

void CommandStack::Redo()
{
    if (CanRedo())
    {
        Command2* commandToRedo = GetCommandInternal(nextCommandIndex);
        nextCommandIndex++;

        if (nullptr != commandToRedo)
        {
            commandToRedo->Redo();
            EmitNotify(commandToRedo, true);
        }
    }

    CleanCheck();
}

void CommandStack::Exec(Command2::Pointer&& command)
{
    DVASSERT(command);

    if (command->CanUndo())
    {
        if (curBatchCommand != nullptr)
        {
            curBatchCommand->AddAndExec(std::move(command));
        }
        else
        {
            ExecInternal(std::move(command), true);
        }
    }
    else
    {
        command->Redo();
        EmitNotify(command.get(), true);
    }
}

void CommandStack::BeginBatch(const DAVA::String& text, DAVA::uint32 commandsCount)
{
    if (nestedBatchesCounter++ == 0)
    {
        curBatchCommand.reset(new CommandBatch(text, commandsCount));
        curBatchCommand->SetNotify(this);
    }
    else
    {
        DVASSERT(curBatchCommand);
    }
}

void CommandStack::EndBatch()
{
    if (curBatchCommand)
    {
        --nestedBatchesCounter;
        DVASSERT(nestedBatchesCounter >= 0);

        if (nestedBatchesCounter > 0)
            return;

        if (curBatchCommand->Empty())
        {
            curBatchCommand.reset();
        }
        else
        {
            // all command were already executed in batch
            // so just add them to stack without calling redo
            ExecInternal(std::move(curBatchCommand), false);
        }
    }
}

bool CommandStack::IsClean() const
{
    if (nextAfterCleanCommandIndex == INVALID_CLEAN_INDEX)
    {
        return false;
    }

    DAVA::int32 startCommandIndex = DAVA::Min(nextAfterCleanCommandIndex, nextCommandIndex);
    DAVA::int32 endCommandIndex = DAVA::Max(nextAfterCleanCommandIndex, nextCommandIndex);

    for (DAVA::int32 i = startCommandIndex; i < endCommandIndex; ++i)
    {
        Command2* command = GetCommandInternal(i);
        DVASSERT(command != nullptr);
        if (command->IsModifying())
            return false;
    }

    return true;
}

void CommandStack::SetClean(bool clean)
{
    if (clean)
    {
        nextAfterCleanCommandIndex = nextCommandIndex;
    }
    else
    {
        nextAfterCleanCommandIndex = INVALID_CLEAN_INDEX;
    }

    CleanCheck();
}

DAVA::int32 CommandStack::GetCleanIndex() const
{
    return nextAfterCleanCommandIndex;
}

DAVA::int32 CommandStack::GetNextIndex() const
{
    return nextCommandIndex;
}

DAVA::int32 CommandStack::GetUndoLimit() const
{
    return commandListLimit;
}

void CommandStack::SetUndoLimit(DAVA::int32 limit)
{
    commandListLimit = limit;
}

DAVA::uint32 CommandStack::GetCount() const
{
    return static_cast<DAVA::uint32>(commandList.size());
}

const Command2* CommandStack::GetCommand(DAVA::int32 index) const
{
    return GetCommandInternal(index);
}

Command2* CommandStack::GetCommandInternal(DAVA::int32 index) const
{
    if (index < static_cast<DAVA::int32>(commandList.size()))
    {
        CommandsContainer::const_iterator i = commandList.begin();
        std::advance(i, index);

        if (i != commandList.end())
        {
            return (*i).get();
        }
    }
    else
    {
        DAVA::Logger::Error("In %s requested wrong index %d from %d", __FUNCTION__, index, commandList.size());
    }

    return nullptr;
}

void CommandStack::ExecInternal(Command2::Pointer&& command, bool runCommand)
{
    ClearRedoCommands();

    Command2* actualCommand = command.get();

    commandList.emplace_back(std::move(command));
    nextCommandIndex++;

    if (runCommand)
    {
        actualCommand->SetNotify(this);
        actualCommand->Redo();
    }

    EmitNotify(actualCommand, true);
    ClearLimitedCommands();

    CleanCheck();
}

void CommandStack::ClearRedoCommands()
{
    if (CanRedo())
    {
        if (nextCommandIndex < nextAfterCleanCommandIndex)
        {
            SetClean(false);
        }

        CommandsContainer::iterator i = commandList.begin();
        std::advance(i, nextCommandIndex);
        while (i != commandList.end())
        {
            i = commandList.erase(i);
        }
    }
}

void CommandStack::ClearLimitedCommands()
{
    while ((commandListLimit > 0) && (static_cast<DAVA::int32>(commandList.size()) > commandListLimit))
    {
        commandList.pop_front();

        nextCommandIndex = DAVA::Max(nextCommandIndex - 1, 0);
        nextAfterCleanCommandIndex = DAVA::Max(nextAfterCleanCommandIndex - 1, 0);
    }

    CleanCheck();
}

void CommandStack::CleanCheck()
{
    if (lastCheckCleanState != IsClean())
    {
        lastCheckCleanState = IsClean();
        EmitCleanChanged(lastCheckCleanState);
    }
}

void CommandStack::CommandExecuted(const Command2* command, bool redo)
{
    EmitNotify(command, redo);
}

void CommandStack::Notify(const Command2* command, bool redo)
{
    CommandExecuted(command, redo);
}
