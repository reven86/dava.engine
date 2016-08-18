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
            commands.erase(commands.begin() + currentIndex + 1, commands.end());
        }

        if (isSingleCommand)
        {
            command->Redo();
        }
        commands.push_back(std::move(command));
        SetCurrentIndex(currentIndex + 1);
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
    DVASSERT_MSG(commandBatch, "CommandStack::EndMacro called without BeginMacro");
    DVASSERT_MSG(requestedBatchCount != 0, "CommandStack::EndMacro called without BeginMacro");

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
    }
}

void CommandStack::Redo()
{
    DVASSERT(CanRedo());
    if (CanRedo())
    {
        commands.at(currentIndex + 1)->Redo();
        SetCurrentIndex(currentIndex + 1);
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

DAVA::String CommandStack::GetUndoText() const
{
    if (CanUndo())
    {
        return commands.at(currentIndex)->GetDescription();
    }
    return DAVA::String();
}

DAVA::String CommandStack::GetRedoText() const
{
    if (CanRedo())
    {
        return commands.at(currentIndex + 1)->GetDescription();
    }
    return DAVA::String();
}

void CommandStack::UpdateCleanState()
{
    if (cleanIndex == currentIndex)
    {
        SetClean(true);
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
    SetClean(!containsModifiedCommands);
}

void CommandStack::SetCurrentIndex(int32 currentIndex_)
{
    DVASSERT(currentIndex != currentIndex_);

    int32 oldIndex = currentIndex;
    currentIndex = currentIndex_;

    UpdateCleanState();
    SetCanUndo(CanUndo());
    SetCanRedo(CanRedo());
    undoTextChanged.Emit(GetUndoText());
    redoTextChanged.Emit(GetRedoText());
    currentIndexChanged.Emit(currentIndex, oldIndex);
}

void CommandStack::SetClean(bool isClean_)
{
    if (isClean != isClean_)
    {
        isClean = isClean_;
        cleanChanged.Emit(isClean);
    }
}

void CommandStack::SetCanUndo(bool canUndo_)
{
    if (canUndo != canUndo_)
    {
        canUndo = canUndo_;
        canUndoChanged.Emit(canUndo);
    }
}

void CommandStack::SetCanRedo(bool canRedo_)
{
    if (canRedo != canRedo_)
    {
        canRedo = canRedo_;
        canRedoChanged.Emit(canRedo);
    }
}
}
