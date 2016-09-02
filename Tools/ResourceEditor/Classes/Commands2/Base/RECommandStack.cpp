#include "Commands2/Base/RECommandStack.h"
#include "Commands2/Base/RECommandBatch.h"
#include "Commands2/Base/CommandAction.h"

#include "Commands2/Base/RECommandNotificationObject.h"

RECommandStack::RECommandStack()
    : DAVA::CommandStack()
{
    canRedoChanged.Connect(this, &RECommandStack::CanRedoChanged);
    canUndoChanged.Connect(this, &RECommandStack::CanUndoChanged);
    undoCommandChanged.Connect(this, &RECommandStack::OnUndoCommandChanged);
    redoCommandChanged.Connect(this, &RECommandStack::OnRedoCommandChanged);
    cleanChanged.Connect(this, &CommandNotifyProvider::EmitCleanChanged);
    commandExecuted.Connect(this, &RECommandStack::OnCommandExecuted);
}

RECommandStack::~RECommandStack() = default;

void RECommandStack::Clear()
{
    commands.clear();
    cleanIndex = EMPTY_INDEX;
    SetCurrentIndex(EMPTY_INDEX);
}

void RECommandStack::SetChanged()
{
    CommandStack::EmitCleanChanged(false);
}

void RECommandStack::RemoveCommands(DAVA::uint32 commandId)
{
    for (DAVA::int32 index = static_cast<DAVA::int32>(commands.size() - 1); index >= 0; --index)
    {
        DAVA::Command* commandPtr = commands.at(index).get();
        if (DAVA::IsCommandBatch(commandPtr))
        {
            RECommandBatch* batch = static_cast<RECommandBatch*>(commandPtr);
            batch->RemoveCommands(commandId);
            if (batch->IsEmpty())
            {
                RemoveCommand(index);
            }
        }
        else
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
    DAVA::uint32 size = static_cast<DAVA::uint32>(commands.size());
    for (DAVA::uint32 index = std::max(cleanIndex, 0); index < size; ++index)
    {
        const DAVA::Command* commandPtr = commands.at(index).get();
        if (IsCommandBatch(commandPtr) == false)
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

DAVA::CommandBatch* RECommandStack::CreateCommmandBatch(const DAVA::String& name, DAVA::uint32 commandsCount) const
{
    return new RECommandBatch(name, commandsCount);
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

void RECommandStack::OnCommandExecuted(const DAVA::Command* command, bool redo)
{
    RECommandNotificationObject notification;
    if (DAVA::IsCommandBatch(command))
    {
        notification.batch = static_cast<const RECommandBatch*>(command);
    }
    else
    {
        notification.command = static_cast<const RECommand*>(command);
    }
    notification.redo = redo;
    EmitNotify(notification);
}

void RECommandStack::ExecInternal(std::unique_ptr<DAVA::Command>&& command, bool isSingleCommand)
{
    if (IsCommandAction(command.get()))
    {
        //get ownership of the given command;
        std::unique_ptr<DAVA::Command> commandAction(std::move(command));
        commandAction->Redo();
        OnCommandExecuted(commandAction.get(), true);

        if (!commandAction->IsClean())
        {
            SetChanged();
        }
    }
    else
    {
        CommandStack::ExecInternal(std::move(command), isSingleCommand);
    }
}

void RECommandStack::OnUndoCommandChanged(const DAVA::Command* command)
{
    if (command != nullptr)
    {
        UndoTextChanged(command->GetDescription());
    }
    else
    {
        UndoTextChanged(DAVA::String());
    }
}

void RECommandStack::OnRedoCommandChanged(const DAVA::Command* command)
{
    if (command != nullptr)
    {
        RedoTextChanged(command->GetDescription());
    }
    else
    {
        RedoTextChanged(DAVA::String());
    }
}
