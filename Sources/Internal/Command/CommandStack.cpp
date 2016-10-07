#include "Command/CommandStack.h"
#include "Command/CommandBatch.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
CommandStack::~CommandStack() = default;

void CommandStack::Exec(std::unique_ptr<Command>&& command)
{
    DVASSERT(command);
    if (command)
    {
        ExecInternal(std::move(command), true);
    }
}

void CommandStack::ExecInternal(std::unique_ptr<Command>&& command, bool isSingleCommand)
{
    if (commandBatch)
    {
        DVASSERT(isSingleCommand == true);
        commandBatch->AddAndRedo(std::move(command));
    }
    else
    {
        DVASSERT(requestedBatchCount == 0);

        if (currentIndex != commands.size() - 1)
        {
            commands.erase(commands.begin() + (currentIndex + 1), commands.end());
        }

        if (isSingleCommand)
        {
            command->Redo();
        }
        commands.push_back(std::move(command));
        SetCurrentIndex(currentIndex + 1);
        //invoke it after SetCurrentIndex to discard logic problems, when client code trying to get IsClean, CanUndo or CanRedo after got commandExecuted
        commandExecuted.Emit(commands.back().get(), true);
    }
}

void CommandStack::BeginBatch(const String& name, uint32 commandsCount)
{
    if (requestedBatchCount == 0)
    {
        DVASSERT(!commandBatch);
        commandBatch.reset(CreateCommmandBatch(name, commandsCount));
    }
    DVASSERT(commandBatch);

    ++requestedBatchCount;
}

CommandBatch* CommandStack::CreateCommmandBatch(const String& name, uint32 commandsCount) const
{
    return new CommandBatch(name, commandsCount);
}

void CommandStack::EndBatch()
{
    DVASSERT(commandBatch, "CommandStack::EndMacro called without BeginMacro");
    DVASSERT(requestedBatchCount != 0, "CommandStack::EndMacro called without BeginMacro");

    --requestedBatchCount;
    if (requestedBatchCount == 0)
    {
        CommandBatch* commandBatchPtr = static_cast<CommandBatch*>(commandBatch.get());
        if (commandBatchPtr->IsEmpty())
        {
            commandBatch.reset();
        }
        else
        {
            //we need to release rootBatch before we will do something
            std::unique_ptr<CommandBatch> commandBatchCopy(std::move(commandBatch)); //do not remove this code!
            ExecInternal(std::move(commandBatchCopy), false);
        }
    }
}

bool CommandStack::IsClean() const
{
    return isClean;
}

void CommandStack::SetClean()
{
    cleanIndex = currentIndex;
    UpdateCleanState();
}

void CommandStack::Undo()
{
    DVASSERT(CanUndo());
    if (CanUndo())
    {
        commands.at(currentIndex)->Undo();
        SetCurrentIndex(currentIndex - 1);
        //invoke it after SetCurrentIndex to discard logic problems, when client code trying to get IsClean, CanUndo or CanRedo after got commandExecuted
        commandExecuted.Emit(commands.at(currentIndex + 1).get(), false);
    }
}

void CommandStack::Redo()
{
    DVASSERT(CanRedo());
    if (CanRedo())
    {
        commands.at(currentIndex + 1)->Redo();
        SetCurrentIndex(currentIndex + 1);
        //invoke it after SetCurrentIndex to discard logic problems, when client code trying to get IsClean, CanUndo or CanRedo after got commandExecuted
        commandExecuted.Emit(commands.at(currentIndex).get(), true);
    }
}

bool CommandStack::CanUndo() const
{
    return currentIndex > EMPTY_INDEX;
}

bool CommandStack::CanRedo() const
{
    return currentIndex < (static_cast<int32>(commands.size()) - 1);
}

const Command* CommandStack::GetUndoCommand() const
{
    if (CanUndo())
    {
        return commands.at(currentIndex).get();
    }
    return nullptr;
}

const Command* CommandStack::GetRedoCommand() const
{
    if (CanRedo())
    {
        return commands.at(currentIndex + 1).get();
    }
    return nullptr;
}

void CommandStack::UpdateCleanState()
{
    if (cleanIndex == currentIndex)
    {
        EmitCleanChanged(true);
        return;
    }
    int32 begin = std::min(cleanIndex, currentIndex);
    int32 end = std::max(cleanIndex, currentIndex);
    DVASSERT(end > begin);
    bool containsModifiedCommands = false;
    for (int32 index = begin; index != end && !containsModifiedCommands; ++index)
    {
        //we need to look only next commands after
        const std::unique_ptr<Command>& command = commands.at(index + 1);
        containsModifiedCommands |= (command->IsClean() == false);
    }
    EmitCleanChanged(!containsModifiedCommands);
}

void CommandStack::SetCurrentIndex(int32 currentIndex_)
{
    if (currentIndex != currentIndex_)
    {
        currentIndex = currentIndex_;

        UpdateCleanState();
        EmitCanUndoChanged(CanUndo());
        EmitCanRedoChanged(CanRedo());
        undoCommandChanged.Emit(GetUndoCommand());
        redoCommandChanged.Emit(GetRedoCommand());
    }
}

void CommandStack::EmitCleanChanged(bool isClean_)
{
    if (isClean != isClean_)
    {
        isClean = isClean_;
        cleanChanged.Emit(isClean);
    }
}

void CommandStack::EmitCanUndoChanged(bool canUndo_)
{
    if (canUndo != canUndo_)
    {
        canUndo = canUndo_;
        canUndoChanged.Emit(canUndo);
    }
}

void CommandStack::EmitCanRedoChanged(bool canRedo_)
{
    if (canRedo != canRedo_)
    {
        canRedo = canRedo_;
        canRedoChanged.Emit(canRedo);
    }
}
}
